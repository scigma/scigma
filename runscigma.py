try:
    import scigma
except:
    import traceback
    traceback.print_exc()
    
    print()
    print("Could not import scigma.")

try:
    input = raw_input
except NameError: pass

input("Press enter to exit.")

print("Exiting.")
