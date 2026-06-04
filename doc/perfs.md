# Performances

## Profiling Caprice on Linux

Build with profiling information enabled:
```
make clean
make -j 10 CFLAGS="-pg -fno-omit-frame-pointer" LDFLAGS="-pg"
```

Then run without limiting the speed:
```
./cap32 -O system.limit_speed=0
```

The flat profile of the run can be obtained with:
```
gprof -p ./cap32 gmon.out | head -n 30
```

And the call graph (usually large) with:
```
gprof -q ./cap32 gmon.out | less
```
