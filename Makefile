NAME        = build/expert_system
CXX         = g++
CXXFLAGS    = -std=c++20 -Wall -Wextra -Werror -Iinclude -MMD -MP -g -fsanitize=address

SRCS        = src/main.cpp \
              src/ast/AtomicNode.cpp \
              src/ast/OpNode.cpp \
							src/ast/NotNode.cpp \
              src/ast/ASTVisitor.cpp\
              src/parser/Lexer.cpp \
              src/parser/Parser.cpp
              # uncomment when implemented
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

# every tests/test*.cpp is its own executable, linked with everything but main
TEST_SRCS   = $(wildcard tests/test*.cpp)
TEST_BINS   = $(TEST_SRCS:tests/%.cpp=build/tests/%)
TEST_OBJS   = $(filter-out build/main.o, $(OBJS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/tests/%: tests/%.cpp $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $< $(TEST_OBJS)

# runs every test executable, fails if any of them fails
test: $(TEST_BINS)
	@status=0; for t in $(TEST_BINS); do ./$$t || status=1; done; exit $$status

clean:
	rm -rf $(OBJS) $(DEPS) build/tests

run: $(OBJS)
	./build/expert_system

fclean: clean
	rm -f $(NAME)
	rm -rf build

re: fclean all

-include $(DEPS) $(TEST_BINS:=.d)

.PHONY: all clean fclean re test
