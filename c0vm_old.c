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
    /* Local variables (you won't need this till Task 2) */
  (void)V;
  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack;
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
      int retval = val2int(c0v_pop(S));
      assert(c0v_stack_empty(S));
#ifdef DEBUG
      fprintf(stderr, "Returning %d from execute()\n", retval);
#endif
      // Free everything before returning from the execute function!
      return retval;
    }


    /* Arithmetic and Logical operations */

    case IADD:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      uint32_t v_uint32_1;
      int32_t v_int32_2;
      uint32_t v_uint32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
        v_uint32_1 = (uint32_t)v_int32_1;
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
        v_uint32_2 = (uint32_t)v_int32_2;
      }
  
      if(v_1.kind == C0_POINTER){      
        void *v_1_ptr = val2ptr(v_1);
        v_uint32_1 = *(uint32_t*)v_1_ptr;
      }

      if(v_2.kind == C0_POINTER){
        void *v_2_ptr = val2ptr(v_2);
        v_uint32_2 = *(uint32_t*)v_2_ptr;
      }

      uint32_t result_uint32 = v_uint32_1 + v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case ISUB:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      uint32_t v_uint32_1;
      int32_t v_int32_2;
      uint32_t v_uint32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
        v_uint32_1 = (uint32_t)v_int32_1;
        printf("%x, %d\n",v_uint32_1,v_uint32_1);
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
        v_uint32_2 = (uint32_t)v_int32_2;
        printf("%x, %d\n",v_uint32_2,v_uint32_2);
      }
  
      if(v_1.kind == C0_POINTER){      
        void *v_1_ptr = val2ptr(v_1);
        v_int32_1 = *(int32_t*)v_1_ptr;
        v_uint32_1 = (uint32_t)v_int32_1;
        printf("%x, %d\n",v_uint32_1,v_uint32_1);
      }

      if(v_2.kind == C0_POINTER){
        void *v_2_ptr = val2ptr(v_2);
        v_int32_2 = *(int32_t*)v_2_ptr;
        v_uint32_2 = (uint32_t)v_int32_2;
        printf("%x, %d\n",v_uint32_2,v_uint32_2);
      }

      uint32_t result_uint32 = v_uint32_2 - v_uint32_1;
      int32_t result_int32 = (int32_t)result_uint32;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);

      break;
    }

    case IMUL:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      c0_value v_2 = c0v_pop(S);

      int32_t v_int32_1;
      uint32_t v_uint32_1;
      int32_t v_int32_2;
      uint32_t v_uint32_2;
      if(v_1.kind == C0_INTEGER){      
        v_int32_1 = val2int(v_1);
        v_uint32_1 = (uint32_t)v_int32_1;
      }

      if(v_2.kind == C0_INTEGER){
        v_int32_2 = val2int(v_2);
        v_uint32_2 = (uint32_t)v_int32_2;
      }
  
      if(v_1.kind == C0_POINTER){      
        void *v_1_ptr = val2ptr(v_1);
        v_uint32_1 = *(uint32_t*)v_1_ptr;
      }

      if(v_2.kind == C0_POINTER){
        void *v_2_ptr = val2ptr(v_2);
        v_uint32_2 = *(uint32_t*)v_2_ptr;
      }

      uint32_t result_uint32 = v_uint32_1 * v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
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

      if(v_1.kind == C0_POINTER){
        void *v_1_ptr = val2ptr(v_1);
        v_int32_1 = *(int32_t*)v_1_ptr;
        if(v_int32_1 == 0){
            c0_arith_error("divide by zero\n");
        }
      }

      if(v_2.kind == C0_POINTER){
        void *v_2_ptr = val2ptr(v_2);
        v_int32_2 = *(int32_t*)v_2_ptr;
        if((v_int32_1 == (int32_t)-1 && 
        (v_int32_2 == (int32_t)0x80000000))){
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
      int32_t v_int32_1 = val2int(v_1);

      if(v_int32_1 == 0){
        c0_arith_error("divide by zero\n");
      }
      c0_value v_2 = c0v_pop(S);
      int32_t v_int32_2 = val2int(v_2);

      if(v_int32_1 == (int32_t)-1 && (v_int32_2 == (int32_t)0x80000000)){
        c0_arith_error("int_min/-1 overflow\n");
      }

      int32_t result_int32 = v_int32_2 % v_int32_1;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);
      break;
    }

    case IAND:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t v_int32_1 = val2int(v_1);
      uint32_t v_uint32_1 = (uint32_t)v_int32_1;
      c0_value v_2 = c0v_pop(S);
      int32_t v_int32_2 = val2int(v_2);
      uint32_t v_uint32_2 = (uint32_t)v_int32_2;
      uint32_t result_uint32 = v_uint32_1 & v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);
      break;
    }

    case IOR:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t v_int32_1 = val2int(v_1);
      uint32_t v_uint32_1 = (uint32_t)v_int32_1;
      c0_value v_2 = c0v_pop(S);
      int32_t v_int32_2 = val2int(v_2);
      uint32_t v_uint32_2 = (uint32_t)v_int32_2;
      uint32_t result_uint32 = v_uint32_1 | v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);
      break;      
    }

    case IXOR:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t v_int32_1 = val2int(v_1);
      uint32_t v_uint32_1 = (uint32_t)v_int32_1;
      c0_value v_2 = c0v_pop(S);
      int32_t v_int32_2 = val2int(v_2);
      uint32_t v_uint32_2 = (uint32_t)v_int32_2;
      uint32_t result_uint32 = v_uint32_1 ^ v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);
      break;    
    }

    case ISHL:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t v_int32_1 = val2int(v_1);
      uint32_t v_uint32_1 = (uint32_t)v_int32_1;
      c0_value v_2 = c0v_pop(S);
      int32_t v_int32_2 = val2int(v_2);
      uint32_t v_uint32_2 = (uint32_t)v_int32_2;
      uint32_t result_uint32 = v_uint32_1 << v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
      c0_value result = int2val(result_int32);
      c0v_push(S, result);
      break;   
    }

    case ISHR:{
      pc++;
      c0_value v_1 = c0v_pop(S);
      int32_t v_int32_1 = val2int(v_1);
      uint32_t v_uint32_1 = (uint32_t)v_int32_1;
      c0_value v_2 = c0v_pop(S);
      int32_t v_int32_2 = val2int(v_2);
      uint32_t v_uint32_2 = (uint32_t)v_int32_2;
      uint32_t result_uint32 = v_uint32_1 >> v_uint32_2;
      int32_t result_int32 = (int32_t)result_uint32;
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
      uint16_t pos = P[pc] << 8;
      pc++;
      pos =  pos|P[pc];

      c0_value push_c0_value = ptr2val(bc0->int_pool+pos);
      c0v_push(S, push_c0_value);
      pc++;
      break;      
    }

    case ALDC:{
      pc++;
      uint16_t pos = P[pc] << 8;
      pc++;
      pos =  pos|P[pc];

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
      int32_t i_op = (int8_t)P[pc];
      c0_value push_c0_value = V[i_op];
      c0v_push(S, push_c0_value);
      pc++;
      break;
    }

    case VSTORE:{
      pc++;
      int32_t i_op = (int8_t)P[pc];
      c0_value pop_c0_value = c0v_pop(S);

      V[i_op] = pop_c0_value;
      pc++;
      break;
    }


    /* Control flow operations */

    case NOP:

    case IF_CMPEQ:

    case IF_CMPNE:

    case IF_ICMPLT:

    case IF_ICMPGE:

    case IF_ICMPGT:

    case IF_ICMPLE:

    case GOTO:

    case ATHROW:

    case ASSERT:


    /* Function call operations: */

    case INVOKESTATIC:

    case INVOKENATIVE:


    /* Memory allocation operations: */

    case NEW:

    case NEWARRAY:

    case ARRAYLENGTH:


    /* Memory access operations: */

    case AADDF:

    case AADDS:

    case IMLOAD:

    case IMSTORE:

    case AMLOAD:

    case AMSTORE:

    case CMLOAD:

    case CMSTORE:

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
