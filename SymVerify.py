# Useing sympy for this, because birds arnt real
from sympy import *
import re 

# Check if code is symbolically equal to correct
def Check_Symbolically(correct, code):
    # Make the code symbol list
    codeSymList = []
    for line in code:
        # Use regex to delete all non numbers on a line and append it
        codeSymList.append(re.sub("\D", "", line))

    # Go though the codes symbol list
    for line in codeSymList:
        # Get the symbol for this line
        symbol = symbols(line[0] +'='+line[1])
        # Check if it is in the correct symbols, if so remove it
        if symbol in correct:
            correct.remove(symbol)
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

########################
##     Test inputs    ##
########################
# Set what the correct list of symbols is
correct_in = list(symbols(
                '0=1 '
                '1=2 '
                '2=3 '
                '3=0 '
                '4=4 '
                '5=5 '
                '6=6 '
                '7=7 '))

# Read in the code, handwaving it for now
code_in = ["dst[2] = src[3];",
        "dst[0] = src[1];",
        "dst[1] = src[2];",
        "dst[3] = src[0];",
        "dst[4] = src[4];",
        "dst[5] = src[5];",
        "dst[6] = src[6];",
        "dst[7] = src[7];",]

print(Check_Symbolically(correct_in, code_in))