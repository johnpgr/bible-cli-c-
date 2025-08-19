#include "allocator.h"
#include "cli.h"

int main(int argc, char* argv[]) {
    ArenaAllocator arena = ArenaAllocator::init(PageAllocator::init(), 4096, MB(8));
    Allocator allocator = arena.allocator();
    defer { arena.deinit(); };

    while(true){ 
        std::println("Total Size: {} Allocated: {}", arena.max_size, arena.total_allocated);
        if(!allocator.create<i32>()) {
            std::println("Failed to allocate.");
            exit(1);
        }
    }

    // CLIParser parser = CLIParser::init(allocator, "Bible Reader");
    // defer { parser.deinit(allocator); };
    //
    // parser.parse(argc, argv);

    return 0;
}
