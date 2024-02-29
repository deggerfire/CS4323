import sys



def main():
    print("void permute(float *dst, float *src)\n{")
    # Get the command-line arguments
    args = sys.argv[1:]

    # Print each argument
    for arg in args:
        print(arg)
    print("}")

if __name__ == "__main__":
    main()
