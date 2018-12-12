#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Function body */
  size_t pc;     /* Program counter */
  c0_value *V;   /* The local variables */
};

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S = c0v_stack_new(); /* Operand stack of C0 values */
  ubyte *P = bc0->function_pool[0].code;      /* Array of bytes that make up the current function */
  size_t pc = 0;     /* Current location within the current byte array P */
  size_t V_size = bc0->function_pool->num_vars;
  c0_value *V = xmalloc(sizeof(c0_value)*V_size); 
//  printf("local variable size is: %d \n",(int32_t)V_size);  
    /* Local variables (you won't need this till Task 2) */
  (void)V;
  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack = stack_new();
  (void) callStack;

  while (true) {

#ifdef DEBUG
    /* You can add extra debugging information here */
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP:{
      pc++;
      c0_value v1 = c0v_pop(S);
      c0_value v2 = c0v_pop(S);
      c0v_push(S,v1);
      c0v_push(S,v2);
      break;
    }


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to be revise it further when you write INVOKESTATIC. */

    case RETURN: {
      c0_value ret_val = c0v_pop(S);

      assert(c0v_stack_empty(S));

      // Free everything before returning from the execute function!
      c0v_stack_free(S);
      free(V);
      if(stack_empty(callStack)){
        stack_free(callStack,NULL);
        int retval = val2int(ret_val);
        #ifdef DEBUG
          fprintf(stderr, "Returning %d from execute()\n", retval);
        #endif
        return retval;
      }else{

        frame* frm =(frame*)pop(callStack);
        S = frm->S;
        P = frm->P;
        V = frm->V;
        pc = frm->pc;

        free(frm); 
        c0v_push(S, ret_val);
        }
        break;
    }


    /* Arithmetic and Logical operations */

    case IADD:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }
  
      int32_t result_int32 = v_int32_1 + v_int32_2;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case ISUB:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }

      int32_t result_int32 = v_int32_2 - v_int32_1;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case IMUL:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }
  

      int32_t result_int32 = v_int32_1 * v_int32_2;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;

    }

    case IDIV:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){
        v_int32_1 = val2int(v_1);
        if(v_int32_1 == 0){
            c0_arith_error("divide by zero\n");
        }
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
        if(v_int32_1 == (int32_t)-1 && (v_int32_2 == (int32_t)0x80000000)){
            c0_arith_error("int_min/-1 overflow\n");
        }
      }

      int32_t result_int32 = v_int32_2 / v_int32_1;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case IREM:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){
        v_int32_1 = val2int(v_1);
        if(v_int32_1 == 0){
            c0_arith_error("divide by zero\n");
        }
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
        if(v_int32_1 == (int32_t)-1 && (v_int32_2 == (int32_t)0x80000000)){
            c0_arith_error("int_min/-1 overflow\n");
        }
      }

      int32_t result_int32 = v_int32_2 % v_int32_1;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case IAND:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }
  
      int32_t result_int32 = v_int32_1 & v_int32_2;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case IOR:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }

      int32_t result_int32 = v_int32_1 | v_int32_2;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;      
    }

    case IXOR:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }

      int32_t result_int32 = v_int32_1 ^ v_int32_2;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;   
    }

    case ISHL:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }
  
      if(v_int32_1 < 0 || v_int32_1 >= 32){
        c0_arith_error("shift negative or more than 32 bits\n");        
      }
      int32_t result_int32 = v_int32_2 << v_int32_1;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case ISHR:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      int32_t v_int32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
      }

      if(v_int32_1 < 0 || v_int32_1 >= 32){
        c0_arith_error("shift negative or more than 32 bits\n");        
      }

      int32_t result_int32 = v_int32_2 >> v_int32_1;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }


    /* Pushing constants */

    case BIPUSH:{
      pc++;
      int32_t push_int32 = (int8_t)P[pc];

      c0_value push_c0_value = int2val(push_int32);
      c0v_push(S, push_c0_value);
      pc++;
      break;
    }

    case ILDC:{
      pc++;
      uint16_t pos = ((uint16_t)P[pc]) << 8;
      pc++;
      pos =  pos|((uint16_t)P[pc]);

      c0_value push_c0_value = int2val(bc0->int_pool[pos]);
      c0v_push(S, push_c0_value);
      pc++;
      break;      
    }

    case ALDC:{
      pc++;
      uint16_t pos = ((uint16_t)P[pc]) << 8;
      pc++;
      pos =  pos|((uint16_t)P[pc]);

      c0_value push_c0_value = ptr2val(bc0->string_pool+pos);
      c0v_push(S, push_c0_value);
      pc++;
      break;  
    }

    case ACONST_NULL:{
      pc++;
      void* ptr = NULL;
      c0_value ptr_c0 = ptr2val(ptr);
      c0v_push(S, ptr_c0);
      break;
    }


    /* Operations on local variables */

    case VLOAD:{
      pc++;
      uint32_t i_op = (uint32_t)P[pc];

      if( (size_t)i_op >= V_size){
        c0_memory_error("out of boundary\n");
      }

      c0_value push_c0_value = V[i_op];
      c0v_push(S, push_c0_value);
      pc++;
      break;
    }

    case VSTORE:{
      pc++;
      uint32_t i_op = (uint32_t)P[pc];

      if( (size_t)i_op >= V_size){
        c0_memory_error("out of boundary\n");
      }

      c0_value pop_c0_value = c0v_pop(S);

      V[i_op] = pop_c0_value;
      pc++;
      break;
    }


    /* Control flow operations */

    case NOP:{
      pc++;
      break;
    }

    case IF_CMPEQ:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(val_equal(v_1,v_2) ){
        int16_t pos = ((int16_t)P[pc]) << 8;
        pc++;
        pos =  pos|((int16_t)P[pc]);
        pc = pc + pos -2;
      }else{
        pc = pc + 2;
      }
      break;
    }

    case IF_CMPNE:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(!val_equal(v_1,v_2) ){
        int16_t pos = ((int16_t)P[pc]) << 8;
        pc++;
        pos =  pos|((int16_t)P[pc]);
        pc = pc + pos -2;
      }else{
        pc = pc + 2;
      }
      break;
    }

    case IF_ICMPLT:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(val2int(v_1) > val2int(v_2)) {
        int16_t pos = ((int16_t)P[pc]) << 8;
        pc++;
        pos =  pos|((int16_t)P[pc]);
        pc = pc + pos -2;
      }else{
        pc = pc + 2;
      }
      break;
    }

    case IF_ICMPGE:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(val2int(v_1) <= val2int(v_2)) {
        int16_t pos = ((int16_t)P[pc]) << 8;
        pc++;
        pos =  pos|((int16_t)P[pc]);
        pc = pc + pos -2;
      }else{
        pc = pc + 2;
      }
      break;
    }

    case IF_ICMPGT:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(val2int(v_1) < val2int(v_2)) {
        int16_t pos = ((int16_t)P[pc]) << 8;
        pc++;
        pos =  pos|((int16_t)P[pc]);
        pc = pc + pos -2;
      }else{
        pc = pc + 2;
      }
      break;
    }

    case IF_ICMPLE:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(val2int(v_1) >= val2int(v_2)) {
        int16_t pos = ((int16_t)P[pc]) << 8;
        pc++;
        pos =  pos|((int16_t)P[pc]);
        pc = pc + pos-2;
        
      }else{
        pc = pc + 2;
      }
      break;
    }

    case GOTO:{
      pc++;
      
      int16_t pos = ((int16_t)P[pc]) << 8;
      pc++;
      pos =  pos|((int16_t)P[pc]);
      pc = pc + pos -2;
      
      break;
    }

    case ATHROW:{
      pc++;
      c0_value v_1 = c0v_pop(S);

      c0_user_error((char*)val2ptr(v_1));
      break;

    }

    case ASSERT:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);
      if(val2int(v_2) == 0) {
        c0_assertion_failure((char*)val2ptr(v_1));
      }
      break;
    }


    /* Function call operations: */

    case INVOKESTATIC:{
      frame* cur_frm = xmalloc(sizeof(frame));

      cur_frm->S = S;      
      cur_frm->P = P;
      cur_frm->V = V;

      push(callStack, (void*)cur_frm); 

      printf("stored pc is: %zu\n",pc);

      pc++;      
      uint16_t pos = (uint16_t)P[pc] << 8;
      pc++;
      cur_frm->pc = pc + 1;
      pos =  pos|(uint16_t)P[pc];
      struct function_info fi = bc0->function_pool[pos];

      V = xmalloc(sizeof(c0_value)*fi.num_vars);

      for(uint16_t i = 0; i < fi.num_args; i++){
        c0_value v = c0v_pop(S);
        V[fi.num_args-1-i] = v;
      }

      S = c0v_stack_new();
      P = fi.code;
      pc = 0;  
      break;
    }

    case INVOKENATIVE:{

      pc++;      
      uint16_t pos = (uint16_t)P[pc] << 8;
      pc++;
      pos =  pos|(uint16_t)P[pc];
      struct native_info fi_native_info = bc0->native_pool[pos];

      c0_value* V1 = xmalloc(sizeof(c0_value)*fi_native_info.num_args);

      for(uint16_t i = 0; i < fi_native_info.num_args; i++){
        c0_value v = c0v_pop(S);
        V1[fi_native_info.num_args-1-i] = v;
      }

      native_fn* fi_native = native_function_table[fi_native_info.function_table_index];
      c0_value return_value = (*fi_native)(V1);
      
      pc++; 
      c0v_push(S, return_value);
      free(V1); 
      break;
}


    /* Memory allocation operations: */

    case NEW:{
      pc++;
      uint8_t alloc_size = P[pc];
      char* alloc_ptr = xcalloc(sizeof(char), alloc_size);
      c0v_push(S, ptr2val((void*)alloc_ptr));
      pc++;
      break;
    }

    case NEWARRAY:{
      pc++;
      uint8_t elt_size = P[pc];
      c0_value v = c0v_pop(S);
      int32_t count = (int32_t)val2int(v);
      if(count < 0) c0_assertion_failure("array cnt is less than 0\n"); 
      c0_array* newarray = xcalloc(sizeof(c0_array),1);
      newarray->count = count + 1;
      newarray->elt_size = elt_size;
      char* array = xcalloc(sizeof(char),(count+1)*(int32_t)elt_size);
      newarray->elems = (void*)array;
      c0_value value = ptr2val((void*)newarray);
      c0v_push(S, value);
      pc++;      
      break;
    }

    case ARRAYLENGTH:{
      pc++;
      c0_value v = c0v_pop(S);
      void * v_ptr = val2ptr(v);
      c0_array* array = (c0_array*)v_ptr;
      int32_t cnt = array->count;
      c0_value value = int2val(cnt); 
      c0v_push(S, value);     
      break;
    }


    /* Memory access operations: */

    case AADDF:{
      pc++;
      uint8_t off_size = P[pc];
      pc++;
      c0_value v = c0v_pop(S);
      void *ptr_prev = val2ptr(v);
      if(ptr_prev == NULL) c0_memory_error("accessed a null pointer\n");      
      void *ptr_aft = (void*)((char*)ptr_prev+off_size);
      c0v_push(S, ptr2val(ptr_aft)); 
      break;
    }

    case AADDS:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t i = val2int(v_1);
      c0_value v_2 = c0v_pop(S);
      void * arr = val2ptr(v_2);
      if(arr == NULL) c0_memory_error("accessed a null pointer\n");
      c0_array* array = (c0_array*)arr;
      int count = array->count;
      if(i< 0 || i >= count) c0_memory_error("AADDS: out of boundary array\n");
      int off = i * array->elt_size;
/*
      printf("aadds off is: %d\n", off);
      printf("is is: %d\n", i);
      printf("aadds off is: %d\n", array->elt_size);
      printf("array is %d\n",((char*)array->elems)[0]&0xff);
*/
      
      c0v_push(S, ptr2val((void*)((char*)array->elems + off))); 
      break;
    }

    case IMLOAD:{
      pc++;
      c0_value v = c0v_pop(S);
      void * ptr = val2ptr(v);
      if(ptr == NULL) c0_memory_error("accessed a null pointer");
      int32_t value = *((int*)ptr);
      c0v_push(S, int2val(value));
      break;
    }

    case IMSTORE:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t value = val2int(v_1);
      c0_value v_2 = c0v_pop(S); 
      void * ptr = val2ptr(v_2); 
      if(ptr == NULL) c0_memory_error("accessed a null pointer");
      *((int *)ptr) = value;    
      break;
    }

    case AMLOAD:{
      pc++;
      c0_value v = c0v_pop(S);
      void ** ptr = val2ptr(v);
      if(ptr == NULL) c0_memory_error("accessed a null pointer");
      c0v_push(S, ptr2val(*ptr));
      break;
    }

    case AMSTORE:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      void * ptr_1 = val2ptr(v_1); 
      c0_value v_2 = c0v_pop(S); 
      void ** ptr_2 = val2ptr(v_2); 
      if(ptr_2 == NULL) c0_memory_error("accessed a null pointer");
      *ptr_2 = ptr_1;    
      break;
    }

    case CMLOAD:{
      pc++;
      c0_value v = c0v_pop(S);
      void * ptr_cm = val2ptr(v);
      if(ptr_cm == NULL) c0_memory_error("accessed a null pointer");
      int32_t value = (int32_t)(*((char*)ptr_cm));
      c0v_push(S, int2val(value));
      break;
    }

    case CMSTORE:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t value = val2int(v_1);
      c0_value v_2 = c0v_pop(S); 
      void * ptr_cms = val2ptr(v_2); 
      if(ptr_cms == NULL) c0_memory_error("accessed a null pointer");
      *((char *)ptr_cms) = value & 0x7f;    
      break;      
    }

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
