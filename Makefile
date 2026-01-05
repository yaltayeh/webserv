NAME		    		= $(BUILD_PATH)/webserv
CXX 					= c++
CXXFLAGS 				= -Wall -Wextra -Werror -std=c++98 -Iinclude
BUILD_PATH	      		?= ./build

DEPENDENCIES	    	= libhttp
DEPENDENCIES_DIR		= ./dependencies
DEPENDENCIES_FLAGS      = $(foreach lib,$(DEPENDENCIES),-L$(BUILD_PATH)/$(lib)) \
							$(foreach lib,$(patsubst lib%,%, $(DEPENDENCIES)),-l$(lib))
INCLUDE_FLAGS	   		= -Iinclude $(foreach lib,$(DEPENDENCIES),-I$(DEPENDENCIES_DIR)/$(lib)/include)

SRCS		    		=       src/main.cpp
OBJS		   			 = $(SRCS:src/%.cpp=$(BUILD_PATH)/obj/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a)
		$(CXX) $(CXXFLAGS) $(OBJS) $(DEPENDENCIES_FLAGS) $(INCLUDE_FLAGS) -o $(NAME)

$(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a):
		$(MAKE) -C $(DEPENDENCIES_DIR)/$(basename $(@F)) BUILD_PATH=../../$(BUILD_PATH)/$(basename $(@F)) all
	
$(BUILD_PATH)/obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_PATH)/obj
	@rm -rf $(BUILD_PATH)/lib*/obj

fclean: clean
	@rm -rf $(NAME)
	@rm -rf $(BUILD_PATH)

re: fclean all

.PHONY: all clean re fclean
