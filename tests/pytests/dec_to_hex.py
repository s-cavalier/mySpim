from sys import argv

if __name__ == "__main__":
    assert(len(argv) == 2)
    print(hex(int(argv[1])))