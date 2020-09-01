TARGET := Test.out

C_COURCE := decoding.c
C_COURCE += encoding.c
C_COURCE += main_test.cpp

FLAGS := -Wall
FLAGS += -Wextra

.DEFAULT_GOAL: $(TARGET)

$(TARGET): $(C_COURCE)
	g++ -o $@ $(FLAGS) $^

.PHONY: clean
clean:
	rm $(TARGET)