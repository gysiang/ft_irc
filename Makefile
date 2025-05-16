CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I include -MMD -MP -g

TARGET = ircserv

LIBS = -lcrypto

# Define source and object directories
SRC_DIR = src
OBJ_DIR = obj

SRCS = main.cpp \
		utils.cpp \
		server.cpp \
		client.cpp \
		replies.cpp \
		errors.cpp \
		channel.cpp

# Add "src/" prefix for compilation
SRCS_PATH = $(addprefix $(SRC_DIR)/, $(SRCS))

# Generate object file names
OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Dependency files
DEPS = $(OBJS:.o=.d)

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Include dependency files if they exist
-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
