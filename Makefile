CPPFLAGS := -D _DEBUG -g3 -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -pie -fPIE -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

LOG    := lib/log/src/LOG.cpp
FRONT  := lib/frontend/src/frontend.cpp
TREE   := lib/tree/src/tree.cpp
FILEW  := lib/filework/src/filework.cpp
STD    := lib/standart/src/standart.cpp
ELF    := lib/elf/src/genelf.cpp
MIDDLE := lib/middleend/src/diff.cpp
v86    := lib/backend/src/backend.cpp lib/backend/src/stack.cpp
X86    := lib/bintrans/src/bintrans.cpp lib/bintrans/src/stack.cpp

SRC := $(TREE) $(FILEW) $(STD) $(LOG)

front:
	g++ $(CPPFLAGS)	$(FRONT)  $(SRC) -o run/front

middle:
	g++ $(CPPFLAGS)	$(MIDDLE) $(SRC) -o run/middle

v86_back: 
	g++ $(CPPFLAGS)	$(v86)    $(SRC) -o run/v86_back 

x86_back:
	g++ $(CPPFLAGS)	$(X86) $(ELF) $(SRC) -o run/x86_back 

cp:
	g++ $(CPPFLAGS) lib/cpu/src/cpu.cpp lib/cpu/src/stack.cpp $(LOG) -o run/cpu
	
asm: 
	g++ $(CPPFLAGS) lib/cpu/src/asm.cpp $(FILEW) -o run/asm

ast:
	g++ $(CPPFLAGS) src/ast.cpp -o run/ast 

v86:
	g++ $(CPPFLAGS) src/v86.cpp -o run/v86

x86:
	g++ $(CPPFLAGS) src/x86.cpp -o run/x86

unit_test:
	g++ src/tests.cpp -o run/unit_tests

clean:
	rm run/* obj/*


