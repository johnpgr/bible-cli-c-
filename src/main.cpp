#include "allocator.h"
#include "cli.h"

int main(int argc, char* argv[]) {
    ArenaAllocator arena = ArenaAllocator::init(PageAllocator::init());
    Allocator aa = arena.allocator();
    defer { arena.deinit(); };

    CLIParser parser = CLIParser::init(aa, "Bible Reader");
    defer { parser.deinit(aa); };

    parser.parse(argc, argv);

    return 0;
}
