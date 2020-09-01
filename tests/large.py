
def genmod(n):
    print("namespace Test%d" % n, " (")
    print("     data true, false")
    print("")
    print("     namespace F:G (")
    print("         def and =")
    print("             [ true true -> true | _ _ -> false ]")
    print("     )")
    print("")
    print("     namespace I:J (")
    print("         def or =")
    print("             [ false false -> true | _ _ -> true ]")
    print("     )")
    print("")
    print("     def not =")
    print("         [ false -> true | _ -> false ]")
    print("")
    print(")")

for i in range(0, 2000):
    genmod(i)
