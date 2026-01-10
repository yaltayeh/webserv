NAME		    		= $(BUILD_PATH)/webserv
CXX 					= c++
CXXFLAGS 				= -Wall -Wextra -Werror -std=c++98 -Iinclude
BUILD_PATH	      		?= ./build

DEPENDENCIES	    	= libhttp libdir
DEPENDENCIES_DIR		= ./dependencies

#if you need to force load static libraries (macOS)
ifeq ($(shell uname),Darwin)
DEPENDENCIES_FLAGS	  = $(foreach lib,$(DEPENDENCIES),-L$(BUILD_PATH)/$(lib)) \
							$(foreach lib,$(patsubst lib%,%, $(DEPENDENCIES)), -Wl,-force_load,$(BUILD_PATH)/lib$(lib)/lib$(lib).a)
else
DEPENDENCIES_FLAGS      = $(foreach lib,$(DEPENDENCIES),-L$(BUILD_PATH)/$(lib)) \
							$(foreach lib,$(patsubst lib%,%, $(DEPENDENCIES)), -l$(lib))
endif

INCLUDE_FLAGS	   		= -Iinclude -Iinclude/directives $(foreach lib,$(DEPENDENCIES),-I$(DEPENDENCIES_DIR)/$(lib)/include)

SRCS		    		=   src/main.cpp \
							src/Server.cpp \
							src/ConnectionManager.cpp \
							src/EventLoop.cpp		\
							src/directives/HttpDirective.cpp	\
							src/directives/ServerDirective.cpp	\
							src/directives/LocationDirective.cpp	\
							src/directives/ServerNameDirective.cpp	\
							src/directives/ListenDirective.cpp	\
							src/directives/RootDirective.cpp	\
							src/directives/ErrorPageDirective.cpp	\
							src/directives/ClientMaxBodySizeDirective.cpp	\
							src/directives/KeepaliveTimeoutDirective.cpp	\
							src/directives/AllowedMethodsDirective.cpp	\
							src/directives/ReturnDirective.cpp	\
							src/directives/AutoindexDirective.cpp	\
							src/directives/AutoindexFormatDirective.cpp	\
							src/directives/AutoindexLocaltimeDirective.cpp	\
							src/directives/AutoindexExactSizeDirective.cpp	\
							src/directives/IndexDirective.cpp	\
							src/directives/UploadEnableDirective.cpp	\
							src/directives/UploadStoreDirective.cpp	\
							src/directives/CgiPassDirective.cpp	\
							src/directives/CgiParamDirective.cpp	\
							src/directives/CgiExtensionDirective.cpp	\
							src/directives/DenyDirective.cpp	\
							src/directives/LimitExceptDirective.cpp	\
							src/directives/ClientBodyTempPathDirective.cpp	\
							src/directives/ProxyRequestBufferingDirective.cpp	\
							src/directives/AliasDirective.cpp	\

OBJS		   			 = $(SRCS:src/%.cpp=$(BUILD_PATH)/obj/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a)
		$(CXX) $(OBJS) $(DEPENDENCIES_FLAGS) -o $(NAME)

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
