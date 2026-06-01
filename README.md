# ivscii-cpp
A highly customizable image and video (soon™) to ASCII art converter.

### Why not improve the original Python version?
Because i left that one many years ago because of how messy the codebase is and also not very customizable. The height of the art depends on the width of art to maintaion ratio, which makes the ASCII art smaller or bigger than expected. Its also very slow on low-end devices, taking almost an entire second to render one image. But because Python being an interpreted language, the low performance is unavoidable.

### Bored, then an idea to rewrite.
It was 10 in the morning and i suddenly remembered that ivscii exists, and i want to rewrite it. I researched a little bit on libraries to use on the C++ rewrite and came across [stb](https://github.com/nothings/stb) libraries, which has exactly what i need. And then i decided to go for it. 3 hours of coding and another 4 hours of debugging problems later, i got a working program. And then i decided to compare it against the original version and the performance difference is big. So with that reason, i decided to continue with this project.

### Comparisons
#### Original Python version
![Python screenshot](media/bench_python.png)
Render time: 378ms

#### C++
![C++ screenshot optimized](media/bench_cpp_optimized.png)
Compiler options: `-O2`
Render time: 60ms

#### C++ (aggresive optimization)
![C++ screenshot aggresive](media/bench_cpp_aggresive.png)
Compiler options: `-O3 -march=native -pipe -fno-plt`
Render time: 57ms

#### Conclusion
Its almost 6x faster!!!!

### Examples
![osage-chan in ASCII form](media/example.png)
Render resolution: 960x197
Render time: 68ms (Alacritty)
> NOTE: Render time might vary depending on the terminal. ivscii-cpp recommends a GPU accelerated terminal, although any terminal will also work but with lower performance

### Installation
```
git clone https://github.com/hithere-at/ivscii-cpp
cd ivscii-cpp
g++ -o ivscii -lm -O2 -march=native ivscii.cpp
```

Now execute the program by running:
```
./ivscii
```

### Credits
- [stb libraries](https://github.com/nothings/stb)
