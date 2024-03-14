import sys
from sympy import *
import re 

DEBUG = False

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
    def __init__(self) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)


    def code(self) -> str:
        return ""


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


SCALAR_INSTRUCTIONS = [ Load(), Store() ]


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
                                             shuffle_len: int,
                                             stores_completed: int) -> list:

    if seq_len == 0 \
        or len(inst_pool) == 0 \
        or shuffle_len == stores_completed:
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
        actual_inst = Nop()
        new_stores_completed = stores_completed
        new_order = order
        new_variables_online = variables_online

    assert actual_inst is not None
    assert new_order is not None
    assert new_variables_online is not None

    inst_code = actual_inst.code()

    selected_combinations = generate_scalar_instruction_combinations(inst_pool, new_variables_online,
                                                                     seq_len - 1, new_order, shuffle_len,
                                                                     new_stores_completed)

    updated_selected_combinations = []

    if not isinstance(actual_inst, Nop):
        if selected_combinations:
            for combination in selected_combinations:
                updated_selected_combinations.append(inst_code + '\n' + combination)
        else:
            updated_selected_combinations = [ inst_code + '\n' ]

    # don't select the instruction
    unselected_combinations = generate_scalar_instruction_combinations(inst_pool[1:], variables_online,
                                                                       seq_len, order, shuffle_len,
                                                                       stores_completed)


    total_combinations = []

    total_combinations.extend(updated_selected_combinations)
    total_combinations.extend(unselected_combinations)

    return total_combinations


def generate_all_scalar_instruction_combinations(order: list) -> list:
    max_scalar_inst_len = 3 * len(order)

    for seq_len in range(1, max_scalar_inst_len):
        combinations = generate_scalar_instruction_combinations(list(SCALAR_INSTRUCTIONS), [], seq_len, order, len(order), 0)

        print('')
        print(f'============== {seq_len} ===============')

        for (idx, combination) in enumerate(combinations):
            print(f'----------- {idx} ----------')
            print(combination)

        print('=================================')


def main():
    # Get the command-line arguments
    order = [ int(arg) for arg in sys.argv[1:] ]

    generate_all_scalar_instruction_combinations(order)

# Genrates the symbols for 1 line of code
def Symbol_Gen(line):
    # Clear all the white space
    line = re.sub("\s", "", line)
    if DEBUG: print(line) # Test print so you can get indexs
    # For a dst[a] = src[b] line
    if(line[6] == '='):
        line = re.sub("\D", "", line)
        return [symbols(line[0] + '=' + line[1])]

# Check if code is symbolically equal to correct
def Check_Symbolically(correct, code):
    # Make the code symbol list
    codeSymList = []
    for line in code:
        # Use Symbol_Gen to get the symbols on the line
        for sym in Symbol_Gen(line):
            codeSymList.append(sym)

    if DEBUG: print(codeSymList) # Use this to see what symbols you made
            
    # Go though the codes symbol list
    for line in codeSymList:
        # Check if it is in the correct symbols, if so remove it
        if line in correct:
            correct.remove(line)
        # Else they are not symbolically equal and we are done
        else:
            print("Not Symbolically Equal")
            return False

    # Check is correct is empty (cause everything should of been removed)
    if len(correct) == 0:
        # If it is they are equal
        print("Symbolically Equal")
        return True
    else:
        # Else they are not equal
        print("Not Symbolically Equal")
        return False

if __name__ == "__main__":
    main()
