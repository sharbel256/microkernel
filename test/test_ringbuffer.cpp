#include <gtest/gtest.h>

#include <kernel/model.hpp>
#include <kernel/ring_buffer.hpp>
#include <memory_resource>

TEST(RingBufferTest, HappyPathPushAndPop) {
  std::pmr::monotonic_buffer_resource arena{1024 * 1024};
  RingBuffer<3> buffer(4, &arena);

  trading::Message msg1{1}, msg2{2}, msg3{3};
  ASSERT_TRUE(buffer.push(trading::Message{1}));
  ASSERT_TRUE(buffer.push(trading::Message{2}));
  ASSERT_TRUE(buffer.push(trading::Message{3}));

  trading::Message result;
  ASSERT_TRUE(buffer.pop(result));
  EXPECT_EQ(result.type, msg1.type);
  ASSERT_TRUE(buffer.pop(result));
  EXPECT_EQ(result.type, msg2.type);
  ASSERT_TRUE(buffer.pop(result));
  EXPECT_EQ(result.type, msg3.type);

  ASSERT_FALSE(buffer.pop(result));
}