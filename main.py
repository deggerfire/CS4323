import sys
import sympy
import re
from collections.abc import Iterator


DEBUG = False

MASK_MAX = 256

KIND_MEM_TO_REG = 1
KIND_REG_TO_REG = 2
KIND_REG_TO_MEM = 3

I_ARR = 1
O_ARR = 2
IO_REG = 3

DATA_TYPE = "float"
IN_ARR = "a"
OUT_ARR = "o"


class Instruction:
    def __init__(self, kind, io_kind) -> None:
        self.kind = kind
        self.io_kind = io_kind

    def code() -> str:
        raise Exception("Illegal instruction")


class Nop(Instruction):
    def __init__(self, comment) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.comment = comment


    def code(self) -> str:
        return f'// nop: {self.comment}'


class Load(Instruction):
    def __init__(self, src_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_MEM_TO_REG, I_ARR)

        self.src_arr = src_arr
        self.offset = offset


    def dst_name(self) -> str:
        return f'{self.src_arr}_{self.offset}'


    def code(self) -> str:
        assert ("" != self.src_arr)

        return f'{DATA_TYPE} {self.dst_name()} = {self.src_arr}[{self.offset}];'


class Store(Instruction):
    def __init__(self, src: str = "", dst_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_REG_TO_MEM, O_ARR)

        self.src = src
        self.dst_arr = dst_arr
        self.offset = offset


    def src_name(self) -> str:
        return self.src


    def code(self) -> str:
        assert ("" != self.dst_arr)
        assert ("" != self.src)

        return f'{DATA_TYPE} {self.dst_arr}[{self.offset}] = {self.src};'


class VecLoad(Instruction):
    LENGTH = 8

    def __init__(self, src_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_MEM_TO_REG, I_ARR)

        self.src_arr = src_arr
        self.offset = offset


    def dst_name(self) -> str:
        return f'{self.src_arr}_{self.offset}'


    def code(self) -> str:
        assert ("" != self.src_arr)
        assert ("float" == DATA_TYPE)

        return f'__m256 {self.dst_name()} = _mm256_loadu_ps(&{self.src_arr}[{self.offset}]);'


class VecStore(Instruction):
    LENGTH = 8

    def __init__(self, src: str = "", dst_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_REG_TO_MEM, O_ARR)

        self.src = src
        self.dst_arr = dst_arr
        self.offset = offset


    def src_name(self) -> str:
        return self.src


    def code(self) -> str:
        assert ("" != self.dst_arr)
        assert ("" != self.src)
        assert ("float" == DATA_TYPE)

        return f'_mm256_storeu_ps(&{self.dst_arr}[{self.offset}], {self.src});'


class VecShuffle(Instruction):
    def __init__(self, src_a: str = "", src_b: str = "", mask: int = 0) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.src_a = src_a
        self.src_b = src_b
        self.mask = mask


    def dst_name(self) -> str:
        return f'{self.src_a}_{self.src_b}_shfl_{self.mask}'


    def code(self) -> str:
        return f'{self.dst_name()} = _mm256_shuffle_ps({self.src_a}, {self.src_b}, {self.mask});'


class VecBlend(Instruction):
    def __init__(self, src_a: str = "", src_b: str = "", mask: int = 0) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.src_a = src_a
        self.src_b = src_b
        self.mask = mask


    def dst_name(self) -> str:
        return f'{self.src_a}_{self.src_b}_blnd_{self.mask}'


    def code(self) -> str:
        return f'{self.dst_name()} = _mm256_blend_ps({self.src_a}, {self.src_b}, {self.mask});'


SCALAR_INSTRUCTIONS = [ Load(), Store() ]
VECTOR_INSTRUCTIONS = [ VecLoad(), VecShuffle(), VecBlend(), VecStore() ]


def prune_inst_of_kind(inst_pool: list, kind: int) -> list:
    new_inst_pool = []

    for inst in inst_pool:
        if inst.kind != kind:
            new_inst_pool.append(inst)

    if inst_pool == new_inst_pool:
        return inst_pool
    else:
        return new_inst_pool


def generate_scalar_instruction_combinations(inst_pool: list,
                                             variables_online: list, seq_len: int,
                                             order: list,
                                             stores_completed: int) -> list:

    if seq_len == 0 \
        or len(inst_pool) == 0:
        return []

    # select the instruction
    inst = inst_pool[0]

    actual_inst = None
    new_order = None
    new_variables_online = None
    new_stores_completed = None

    ## using the selected instruction, generate the actual instruction
    if isinstance(inst, Load) and order:
        actual_load = Load(IN_ARR, order[0])

        new_stores_completed = stores_completed

        new_order = order[1:]

        new_variables_online = []
        new_variables_online.extend(variables_online)
        new_variables_online.append(actual_load.dst_name())

        actual_inst = actual_load
    elif isinstance(inst, Store) and variables_online:
        actual_store = Store(variables_online[0], OUT_ARR, stores_completed)

        new_stores_completed = stores_completed + 1

        new_order = order[:]

        new_variables_online = []
        new_variables_online.extend(variables_online)
        new_variables_online.remove(actual_store.src_name())

        actual_inst = actual_store
    else:
        actual_inst = Nop(f'order: {order}, online: {variables_online}')
        new_stores_completed = stores_completed
        new_order = order
        new_variables_online = variables_online

    assert actual_inst is not None
    assert new_order is not None
    assert new_variables_online is not None

    inst_code = actual_inst.code()

    selected_combinations = generate_scalar_instruction_combinations(inst_pool, new_variables_online,
                                                                     seq_len - 1, new_order,
                                                                     new_stores_completed)

    updated_selected_combinations = []

    if selected_combinations:
        for combination in selected_combinations:
            updated_selected_combinations.append(inst_code + '\n' + combination)
    else:
        updated_selected_combinations = [ inst_code + '\n' ]

    # don't select the instruction
    unselected_combinations = generate_scalar_instruction_combinations(inst_pool[1:], variables_online,
                                                                       seq_len, order,
                                                                       stores_completed)


    total_combinations = []

    total_combinations.extend(updated_selected_combinations)
    total_combinations.extend(unselected_combinations)

    return total_combinations


def generate_vector_instruction_combinations(inst_pool: list,
                                             variables_online: list, seq_len: int,
                                             order: list,
                                             stores_completed: int) -> Iterator[str]:

    if seq_len == 0 \
        or len(inst_pool) == 0:
        yield ''
        return

    # select the instruction
    inst = inst_pool[0]

    actual_insts = None
    new_order = None
    # new_variables_online list of lists
    # such that it will store the list of generated stuff for each inst generated
    new_variables_online = None
    new_stores_completed = None

    ## using the selected instruction, generate the actual instruction
    if isinstance(inst, VecLoad) and order:
        offset = order[0]
        actual_load = VecLoad(IN_ARR, offset)

        new_stores_completed = stores_completed

        new_order = [ o for o in order if o >= (offset + VecLoad.LENGTH) ]

        new_variables_online_element = []
        new_variables_online_element.extend(variables_online)
        new_variables_online_element.append(actual_load.dst_name())

        new_variables_online = [ new_variables_online_element ]

        actual_insts = [ actual_load ]
    elif isinstance(inst, VecShuffle) and variables_online:
        actual_insts = []
        new_variables_online = []

        new_stores_completed = stores_completed
        new_order = order

        for src_a_idx in range(len(variables_online)):
            for src_b_idx in range(src_a_idx, len(variables_online)):
                src_a = variables_online[src_a_idx]
                src_b = variables_online[src_b_idx]

                for mask in range(MASK_MAX):
                    new_variables_online_element = []
                    actual_inst = VecShuffle(src_a, src_b, mask)

                    actual_insts.append(actual_inst)

                    new_variables_online_element.extend(variables_online)
                    new_variables_online_element.append(actual_inst.dst_name())
                    new_variables_online.append(new_variables_online_element)
    elif isinstance(inst, VecBlend) and variables_online:
        actual_insts = []
        new_variables_online = []

        new_stores_completed = stores_completed
        new_order = order

        for src_a_idx in range(len(variables_online)):
            for src_b_idx in range(src_a_idx + 1, len(variables_online)):
                src_a = variables_online[src_a_idx]
                src_b = variables_online[src_b_idx]

                for mask in range(MASK_MAX):
                    new_variables_online_element = []
                    actual_inst = VecBlend(src_a, src_b, mask)

                    actual_insts.append(actual_inst)
                    new_variables_online_element.extend(variables_online)
                    new_variables_online_element.append(actual_inst.dst_name())
                    new_variables_online.append(new_variables_online_element)
    elif isinstance(inst, VecStore) and variables_online:
        # using the stores_completed thingy to measure which one to store to
        # this means that the shuffles are only possible within the 8 elements

        actual_store = VecStore(variables_online[0], OUT_ARR, VecStore.LENGTH * stores_completed)

        new_stores_completed = stores_completed + 1

        new_order = order[:]

        new_variables_online_element = []
        new_variables_online_element.extend(variables_online)
        new_variables_online_element.remove(actual_store.src_name())

        new_variables_online = [ new_variables_online_element ]

        actual_insts = [ actual_store ]
    else:
        actual_nop = Nop(f'order: {order}, online: {variables_online}')
        actual_insts = [ actual_nop ]
        new_stores_completed = stores_completed
        new_order = order
        new_variables_online = [ variables_online ]

    assert actual_insts is not None
    assert new_order is not None
    assert new_variables_online is not None
    assert len(actual_insts) == len(new_variables_online)

    # print(f'actual_insts: {actual_insts}')

    for (idx, actual_inst) in enumerate(actual_insts):
        new_variables_online_element = new_variables_online[idx]
        inst_code = actual_inst.code()

        # print(f'trying: {inst_code}')

        for combination in generate_vector_instruction_combinations(inst_pool, new_variables_online_element,
                                                                    seq_len - 1, new_order, new_stores_completed):
            yield inst_code + '\n' + combination

    # don't select the instruction
    for combination in generate_vector_instruction_combinations(inst_pool[1:], variables_online,
                                                                seq_len, order, stores_completed):

        yield combination


def generate_all_instruction_combinations(order: list) -> list:
    scalar_order = list(order)
    max_scalar_inst_len = 3 * len(scalar_order)

    print('scalar sequences:')

    for seq_len in range(1, max_scalar_inst_len):
        print(f'============== {seq_len} ===============')

        combinations = generate_scalar_instruction_combinations(list(SCALAR_INSTRUCTIONS), [], seq_len, scalar_order, 0)

        for (idx, combination) in enumerate(combinations):
            print(f'----------- {idx} ----------')
            print(combination)

        print('=================================')

    vector_order = list(order)
    vector_order.sort()

    max_vector_inst_len = len(vector_order)

    print('vector sequences:')

    for seq_len in range(1, max_vector_inst_len):
        print(f'============== {seq_len} ===============')

        combinations = generate_vector_instruction_combinations(list(VECTOR_INSTRUCTIONS), [], seq_len, vector_order, 0)

        for (idx, combination) in enumerate(combinations):
            print(f'----------- {idx} ----------')
            print(combination)

        print('=================================')


def main():
    # Get the command-line arguments
    order = [ int(arg) for arg in sys.argv[1:] ]

    generate_all_instruction_combinations(order)


## Check if code is symbolically equal to correct
def Check_Symbolically(correct, instructions):
    # Make the code symbol list
    codeSymList = []
    for instr in instructions:
        # Use Symbol_Gen method to get the symbols from instruction
        for sym in instr.Symbol_Gen():
            codeSymList.append(sym)

    if DEBUG: print(codeSymList) # Use this to see what symbols you made
            
    # Go though the codes symbol list
    for line in codeSymList:
        # Check if it is in the correct symbols, if so remove it
        if line in correct:
            correct.remove(line)
        # Else they are not symbolically equal and we are done
        else:
            if DEBUG: print("Not Symbolically Equal")
            return False

    # Check is correct is empty (cause everything should of been removed)
    if len(correct) == 0:
        # If it is they are equal
        if DEBUG: print("Symbolically Equal")
        return True
    else:
        # Else they are not equal
        if DEBUG: print("Not Symbolically Equal")
        return False


if __name__ == "__main__":
    main()
