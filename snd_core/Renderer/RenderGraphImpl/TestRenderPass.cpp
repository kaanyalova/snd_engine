#include "TestRenderPass.hpp"

void TestRenderPass::setup() {
    add_buffer_attachment("whatever", AttachmentFlags::Read);
}