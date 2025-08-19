#include "allocator.h"
#include "cli.h"

int main(int argc, char* argv[]) {
    constexpr usize BUFSIZE = MB(4);
    u8* buffer = (u8*)malloc(BUFSIZE);
    FixedBufferAllocator fba = FixedBufferAllocator::init(&buffer, BUFSIZE);
    Allocator allocator = fba.allocator();
    defer {
        fba.deinit();
        free(buffer);
    };

    // CLIParser parser = CLIParser::init(allocator, "Bible Reader");
    // defer { parser.deinit(allocator); };
    //
    // parser.parse(argc, argv);

    return 0;
}
