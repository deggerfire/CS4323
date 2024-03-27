from __future__ import annotations
import sys
import sympy
import re
import itertools
from collections.abc import Iterator


DEBUG = False

MASK_MAX = 256
REMOVE_ELEMENTS_AS_ASSIGNED = False

KIND_MEM_TO_REG = 1
KIND_REG_TO_REG = 2
KIND_REG_TO_MEM = 3

I_ARR = 1
O_ARR = 2
IO_REG = 3

DATA_TYPE = "float"
IN_ARR = "a"
OUT_ARR = "o"


def generate_shuffle_masks() -> list:
    masks = []
    two_bit_numbers = list(range(4))

    for combinations in itertools.permutations(two_bit_numbers, len(two_bit_numbers)):
        mask = 0

        for (idx, combination) in enumerate(combinations):
            mask = mask | (combination << (idx * 2))

        masks.append(mask)

    return masks


UNDUMB_SHUFFLE_MASKS = generate_shuffle_masks()


class Instruction:
    def __init__(self, kind, io_kind) -> None:
        self.kind = kind
        self.io_kind = io_kind


    def code() -> str:
        raise Exception("Illegal instruction")


    def Symbol_Gen(self, codeSymList) -> str:
        raise Exception("Illegal Symbol")


class Nop(Instruction):
    def __init__(self, comment) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.comment = comment


    def code(self) -> str:
        return f'// nop: {self.comment}'


    def Symbol_Gen(self, codeSymList) -> str:
        return []


    def gen_symbol(self, input: map) -> map:
        return input


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


    def Symbol_Gen(self, codeSymList) -> str:
        codeSymList.append(symbols(f'{self.dst_name()}={self.offset}'))


    def gen_symbol(self, input: map) -> map:
        symbols = {}

        symbols.update(input)
        symbols[self.dst_name()] = sympy.Symbol(self.dst_name())

        return symbols


class Store(Instruction):
    def __init__(self, src: str = "", dst_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_REG_TO_MEM, O_ARR)

        self.src = src
        self.dst_arr = dst_arr
        self.offset = offset


    def src_name(self) -> str:
        return self.src


    def dst_name(self) -> str:
        return f'{self.dst_arr}_{self.offset}'


    def code(self) -> str:
        assert ("" != self.dst_arr)
        assert ("" != self.src)

        return f'{DATA_TYPE} {self.dst_arr}[{self.offset}] = {self.src};'


    def Symbol_Gen(self, codeSymList) -> str:
        for symbol in codeSymList:
            if symbol.contains(f'{self.src}'):
                codeSymList.append(f'{self.offset}={symbol[2]}')
                codeSymList.remove(symbol)


    def gen_symbol(self, input: map) -> map:
        symbols = {}

        symbols.update(input)

        src = input[self.src_name()]
        dst = sympy.Symbol(self.dst_name())

        dst = src

        symbols[self.dst_name()] = dst

        return symbols


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


    def gen_symbol(self, input: map) -> map:
        symbols = {}

        symbols.update(input)

        for local_offset in range(VecLoad.LENGTH):
            offset = local_offset + self.offset
            symbol_name = f'{self.dst_name()}_{local_offset}'
            symbol = f'{self.src_arr}_{offset}'
            symbols[symbol_name] = sympy.Symbol(symbol)

        return symbols


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


    def gen_symbol(self, input: map) -> map:
        symbols = {}

        symbols.update(input)

        for local_offset in range(VecStore.LENGTH):
            offset = local_offset + self.offset
            symbol_name = f'{self.src}_{local_offset}'

            src = input[symbol_name]

            symbol_name = f'{self.dst_arr}_{offset}'
            dst = sympy.Symbol(symbol_name)

            dst = src

            symbols[symbol_name] = dst

        return symbols


class VecShuffle(Instruction):
    def __init__(self, src_a: str = "", src_b: str = "", mask: int = 0) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.src_a = src_a
        self.src_b = src_b
        self.mask = mask


    def dst_name(self) -> str:
        return f'{self.src_a}_{self.src_b}_shfl_{self.mask}'


    def code(self) -> str:
        assert ("float" == DATA_TYPE)

        return f'__m256 {self.dst_name()} = _mm256_shuffle_ps({self.src_a}, {self.src_b}, {self.mask});'


    def select_4(self, variable_offset: int, input: map, mask_at_offset: int) -> map:
        src_base_offset = (variable_offset // 4) * 4

        src = self.src_a if (0 == (variable_offset // 2) % 2) else self.src_b
        src_symbol = input[f'{src}_{src_base_offset + mask_at_offset}']

        dst_name = f'{self.dst_name()}_{variable_offset}'
        dst_symbol = sympy.symbols(dst_name)

        if 0 > mask_at_offset or mask_at_offset >= 4:
            raise IndexError(f"Illegal mask_at_offset = {mask_at_offset}")

        dst_symbol = src_symbol

        symbols = {}
        symbols.update(input)
        symbols[dst_name] = dst_symbol

        return symbols


    def gen_symbol(self, input: map) -> map:
        symbols = {}

        symbols.update(input)

        for variable_offset in range(VecStore.LENGTH):
            # get the 2 bits at the offset
            local_local_offset = (variable_offset * 2) % 8

            # print(f'local_local_offset: {local_local_offset}')

            mask_extracted = 3 << local_local_offset
            mask_at_offset = (mask_extracted & self.mask) >> local_local_offset

            # print(f'mask_at_offset: {mask_at_offset}')

            symbols = self.select_4(variable_offset, symbols, mask_at_offset)

        return symbols


class VecBlend(Instruction):
    def __init__(self, src_a: str = "", src_b: str = "", mask: int = 0) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.src_a = src_a
        self.src_b = src_b
        self.mask = mask


    def dst_name(self) -> str:
        return f'{self.src_a}_{self.src_b}_blnd_{self.mask}'


    def code(self) -> str:
        assert ("float" == DATA_TYPE)

        return f'__m256 {self.dst_name()} = _mm256_blend_ps({self.src_a}, {self.src_b}, {self.mask});'


    def select_4(self, variable_offset: int, input: map, mask_at_offset: int) -> map:
        src = self.src_b if (mask_at_offset) else self.src_a
        src_symbol = input[f'{src}_{variable_offset}']

        dst_name = f'{self.dst_name()}_{variable_offset}'
        dst_symbol = sympy.symbols(dst_name)

        if 0 != mask_at_offset and 1 != mask_at_offset:
            raise IndexError(f"Illegal mask_at_offset = {mask_at_offset}")

        dst_symbol = src_symbol

        symbols = {}
        symbols.update(input)
        symbols[dst_name] = dst_symbol

        return symbols


    def gen_symbol(self, input: map) -> map:
        symbols = {}

        symbols.update(input)

        for variable_offset in range(VecStore.LENGTH):
            mask_extracted = 1 << variable_offset
            mask_at_offset = (mask_extracted & self.mask) >> variable_offset

            symbols = self.select_4(variable_offset, symbols, mask_at_offset)

        return symbols


class CodeSequence:
    def __init__(self, instructions: list) -> None:
        self.instructions = instructions


    def code(self):
        return '\n'.join(inst.code() for inst in self.instructions)


    def prepend(self, instruction: Instruction) -> CodeSequence:
        new_instructions = []
        new_instructions.append(instruction)
        new_instructions.extend(self.instructions)

        return CodeSequence(new_instructions)

    def create_symbols(self) -> map:
        symbols = {}

        for instruction in self.instructions:
            symbols = instruction.gen_symbol(symbols)
            # print(f'symbols: {symbols}')

        return symbols


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

    selected_combinations = generate_scalar_instruction_combinations(inst_pool, new_variables_online,
                                                                     seq_len - 1, new_order,
                                                                     new_stores_completed)

    updated_selected_combinations = []

    if selected_combinations:
        for combination in selected_combinations:
            updated_selected_combinations.append(combination.prepend(actual_inst))
    else:
        updated_selected_combinations = [ CodeSequence([actual_inst]) ]

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
                                             order: list, stores_expected: int,
                                             stores_completed: int) -> Iterator[CodeSequence]:

    if stores_expected <= stores_completed * VecStore.LENGTH:
        return

    if seq_len == 0 \
        or len(inst_pool) == 0:
        yield CodeSequence([])
        return

    # MUST store all the variables generated
    if seq_len == 1 and variables_online:
        for variable_online in variables_online:
            yield CodeSequence([ VecStore(variable_online, OUT_ARR, VecStore.LENGTH * stores_completed) ])

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

                for mask in UNDUMB_SHUFFLE_MASKS:
                    new_variables_online_element = []
                    actual_inst = VecShuffle(src_a, src_b, mask)

                    actual_insts.append(actual_inst)

                    new_variables_online_element.extend(variables_online)
                    new_variables_online_element.append(actual_inst.dst_name())

                    if REMOVE_ELEMENTS_AS_ASSIGNED:
                        new_variables_online_element.remove(src_a)
                        if src_a != src_b: new_variables_online_element.remove(src_b)

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

                    if REMOVE_ELEMENTS_AS_ASSIGNED:
                        new_variables_online_element.remove(src_a)
                        new_variables_online_element.remove(src_b)

                    new_variables_online.append(new_variables_online_element)
    elif isinstance(inst, VecStore) and variables_online:
        # using the stores_completed thingy to measure which one to store to
        # this means that the shuffles are only possible within the 8 elements

        new_stores_completed = stores_completed + 1

        actual_insts = []
        new_variables_online = []
        new_order = order

        for store_variable in variables_online:
            actual_store = VecStore(store_variable, OUT_ARR, VecStore.LENGTH * stores_completed)

            actual_insts.append(actual_store)

            new_variables_online_element = []
            new_variables_online_element.extend(variables_online)
            new_variables_online_element.remove(actual_store.src_name())

            new_variables_online.append(new_variables_online_element)
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

        # print(f'trying: {inst_code}')

        for combination in generate_vector_instruction_combinations(inst_pool, new_variables_online_element,
                                                                    seq_len - 1, new_order, stores_expected, new_stores_completed):
            yield combination.prepend(actual_inst)

    # don't select the instruction
    for combination in generate_vector_instruction_combinations(inst_pool[1:], variables_online,
                                                                seq_len, order, stores_expected, stores_completed):

        yield combination


def generate_baseline_instruction_combination(order: list) -> CodeSequence:
    instructions = []

    for (idx, ord) in enumerate(order):
        load = Load(IN_ARR, ord)
        store = Store(load.dst_name(), OUT_ARR, idx)

        instructions.append(load)
        instructions.append(store)

    return CodeSequence(instructions)


def compare_symbols(expected_output: map, actual_output: map) -> bool:
    for (expected_name, expected_symbol) in expected_output.items():
        if not expected_name.startswith(OUT_ARR):
            continue

        if expected_name not in actual_output.keys():
            return False

        actual_symbol = actual_output[expected_name]

        if f'{actual_symbol}' != f'{expected_symbol}':
            return False

    return True


def test():
    load = VecLoad(IN_ARR, 0)
    # load2 = VecLoad(IN_ARR, 8)
    # shfl = VecBlend(load.dst_name(), load2.dst_name(), 128)
    store = VecStore(load.dst_name(), OUT_ARR, 0)

    seq = CodeSequence([load, store])

    symbols = seq.create_symbols()

    for (k, v) in symbols.items():
        print(f'{k} = {v}')


def generate_all_instruction_combinations(order: list) -> list:
    # test()
    # exit(1)

    print(f'{len(UNDUMB_SHUFFLE_MASKS)}: {UNDUMB_SHUFFLE_MASKS}')

    scalar_order = list(order)
    max_scalar_inst_len = 2 * len(scalar_order)

    baseline = generate_baseline_instruction_combination(order)

    baseline_symbols = baseline.create_symbols()

    print(f'baseline: {baseline_symbols}')

    print('scalar sequences:')

    for seq_len in range(1, max_scalar_inst_len + 1):
        print(f'============== {seq_len} ===============')

        combinations = generate_scalar_instruction_combinations(list(SCALAR_INSTRUCTIONS), [], seq_len, scalar_order, 0)

        for (idx, combination) in enumerate(combinations):
            if compare_symbols(baseline_symbols, combination.create_symbols()):
                print(f'----------- {idx} ----------')
                print(combination.code())

        print('=================================')

    vector_order = list(order)
    vector_order.sort()

    max_vector_inst_len = 5 # len(vector_order)

    print('vector sequences:')

    for seq_len in range(1, max_vector_inst_len + 1):
        print(f'============== {seq_len} ===============')

        combinations = generate_vector_instruction_combinations(list(VECTOR_INSTRUCTIONS), [], seq_len, vector_order, len(vector_order), 0)

        for (idx, combination) in enumerate(combinations):
            if compare_symbols(baseline_symbols, combination.create_symbols()):
                print(f'----------- {idx} ----------')
                print(combination.code())

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
        instr.Symbol_Gen()

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
