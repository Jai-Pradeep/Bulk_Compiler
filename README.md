

##  Step-by-step: How to run

###  1. Compile (you already did this)

```bash
make
```

This creates:

```bash
compiler
```

---

###  2. Prepare input file

You must have a file like:

```bash
program.bc
```

(Our custom language input)

---

###  3. Run the compiler

###  Basic run (no optimization)

```bash
./compiler < program.bc
```

 Output:

* `output.ir` file created

---

###  With optimization

```bash
./compiler -O2 < program.bc
```

Uses:

* constant folding
* copy propagation
* dead code elimination

---

###  Custom output name

```bash
./compiler -O1 -o myprog < program.bc
```

 Output:

```
myprog.ir
```

---

###  Generate C + executable

```bash
./compiler -O2 --emit-c -o myprog < program.bc
```

 This will:

1. Generate `myprog.c`
2. Compile it using `gcc -fopenmp`
3. Create executable:

```bash
./myprog
```

---

###  CUDA mode (if supported)

```bash
./compiler --cuda -o gpu_prog < program.bc
```

---

##  Example workflow (IMPORTANT for exam)

```bash
make
./compiler -O2 -o test < input.bc
cat test.ir
```

OR:

```bash
./compiler -O2 --emit-c -o test < input.bc
./test
```

## To Run all Test cases at once
```bash
mkdir -p Answers logs

for f in Test/*.bc; do
    name=$(basename "$f" .bc)
    echo "=== Running $name ==="

    ./compiler -O2 --emit-c --cuda -o Answers/$name \
        < "$f" \
        > logs/$name.out \
        2> logs/$name.err

    if [ -s logs/$name.err ]; then
        echo "❌ Error in $name"
    else
        echo "✅ Success: $name"
    fi
done
```
