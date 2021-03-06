/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cartographer/sensor/collator.h"

#include <memory>

#include "cartographer/common/lua_parameter_dictionary_test_helpers.h"
#include "cartographer/common/make_unique.h"
#include "cartographer/common/time.h"
#include "cartographer/sensor/proto/sensor.pb.h"
#include "gtest/gtest.h"

namespace cartographer {
namespace sensor {
namespace {

TEST(Collator, Ordering) {
  Data first("horizontal_laser", sensor::LaserFan{});
  Data second("vertical_laser", sensor::LaserFan{});
  Data third("imu", Data::Imu{});
  Data fourth("horizontal_laser", sensor::LaserFan{});
  Data fifth("vertical_laser", sensor::LaserFan{});
  Data sixth("odometry", Data::Odometry{});

  const std::unordered_set<string> frame_ids = {
      "horizontal_laser", "vertical_laser", "imu", "odometry"};
  std::vector<std::pair<int64, Data>> received;
  Collator collator;
  collator.AddTrajectory(
      0, frame_ids,
      [&received](const int64 timestamp, std::unique_ptr<Data> packet) {
        received.push_back(std::make_pair(timestamp, *packet));
      });

  collator.AddSensorData(0, 100, first.frame_id,
                         common::make_unique<Data>(first));
  collator.AddSensorData(0, 600, sixth.frame_id,
                         common::make_unique<Data>(sixth));
  collator.AddSensorData(0, 400, fourth.frame_id,
                         common::make_unique<Data>(fourth));
  collator.AddSensorData(0, 200, second.frame_id,
                         common::make_unique<Data>(second));
  collator.AddSensorData(0, 500, fifth.frame_id,
                         common::make_unique<Data>(fifth));
  collator.AddSensorData(0, 300, third.frame_id,
                         common::make_unique<Data>(third));

  EXPECT_EQ(3, received.size());
  EXPECT_EQ(100, received[0].first);
  EXPECT_EQ("horizontal_laser", received[0].second.frame_id);
  EXPECT_EQ(200, received[1].first);
  EXPECT_EQ("vertical_laser", received[1].second.frame_id);
  EXPECT_EQ(300, received[2].first);
  EXPECT_EQ("imu", received[2].second.frame_id);

  collator.Flush();

  ASSERT_EQ(6, received.size());
  EXPECT_EQ("horizontal_laser", received[3].second.frame_id);
  EXPECT_EQ(500, received[4].first);
  EXPECT_EQ("vertical_laser", received[4].second.frame_id);
  EXPECT_EQ(600, received[5].first);
  EXPECT_EQ("odometry", received[5].second.frame_id);
}

}  // namespace
}  // namespace sensor
}  // namespace cartographer
