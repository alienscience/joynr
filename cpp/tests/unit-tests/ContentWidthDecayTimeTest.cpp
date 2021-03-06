/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
 * %%
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
 * #L%
 */
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utils/QThreadSleep.h"
#include "joynr/ContentWithDecayTime.h"
#include "joynr/JoynrMessage.h"

#include <QDateTime>

using namespace joynr;

TEST(ContentWithDecayTimeTest, messageWithDecayTime)
{
    JoynrMessage message;
    QDateTime decaytime = QDateTime::currentDateTime().addMSecs(2000);
    ContentWithDecayTime<JoynrMessage> mwdt =  ContentWithDecayTime<JoynrMessage>(message, decaytime);
    EXPECT_TRUE(!mwdt.isExpired());
    EXPECT_GT(mwdt.getRemainingTtl_ms(), 1500);
    EXPECT_LT(mwdt.getRemainingTtl_ms(), 2500);
    EXPECT_EQ(decaytime, mwdt.getDecayTime());
    EXPECT_EQ(message, mwdt.getContent());
    QThreadSleep::msleep(1000);
    EXPECT_GT( mwdt.getRemainingTtl_ms(), 500);
    EXPECT_LT( mwdt.getRemainingTtl_ms(), 1500 );
    EXPECT_TRUE(!mwdt.isExpired());

    QThreadSleep::msleep(1500);
    EXPECT_TRUE(mwdt.isExpired());
    EXPECT_LT(mwdt.getRemainingTtl_ms(), 0 );
}
