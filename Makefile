NAME        = build/expert_system
CXX         = c++
CXXFLAGS    = -std=c++20 -Wall -Wextra -Werror -Iinclude -MMD -MP

SRCS        = src/main.cpp \
              src/ast/AtomicNode.cpp \
              src/ast/OpNode.cpp \
              src/ast/ASTVisitor.cpp 
              # uncomment when implemented
#               src/parser/Lexer.cpp \
#               src/parser/Parser.cpp \
#               src/hypergraph/HypergraphOps.cpp \
#               src/hypergraph/HyperEdge.cpp \
#               src/kb/KnowledgeBaseBuilder.cpp \
#               src/inference/ExpressionParser.cpp \
#               src/inference/InferenceEngine.cpp \
#               src/observer/AuditLogger.cpp \
#               src/observer/StateTracker.cpp \
#               src/cache/CacheManager.cpp \
#               src/cache/Serializer.cpp

# Simple substitution instead of shell/patsubst
OBJS        = $(SRCS:src/%.cpp=build/%.o)
DEPS        = $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

build: $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)
	rm -rf build

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
