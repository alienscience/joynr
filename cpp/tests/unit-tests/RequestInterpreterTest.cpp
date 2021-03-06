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
#include "joynr/InterfaceRegistrar.h"
#include "joynr/vehicle/IGps.h"
#include "joynr/vehicle/GpsRequestInterpreter.h"
#include "joynr/vehicle/GpsRequestCaller.h"
#include "joynr/tests/TestRequestInterpreter.h"
#include "joynr/IRequestInterpreter.h"
#include "joynr/JoynrMessageSender.h"
#include "joynr/MessagingQos.h"
#include "joynr/JoynrMessage.h"
#include "joynr/JoynrMessageFactory.h"
#include "joynr/Dispatcher.h"
#include "joynr/Request.h"
#include "tests/utils/MockObjects.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSharedPointer>

using ::testing::A;
using ::testing::_;

using namespace joynr;

class RequestInterpreterTest : public ::testing::Test {
public:
    RequestInterpreterTest()
        : gpsInterfaceName(vehicle::IGpsBase::getInterfaceName())
    {

    }

protected:
    QString gpsInterfaceName;
};


TEST_F(RequestInterpreterTest, execute_callsMethodOnRequestCaller) {
    QSharedPointer<MockTestRequestCaller> mockCaller(new MockTestRequestCaller());
    EXPECT_CALL(*mockCaller,
                getLocation(A<RequestStatus&>(), A<types::GpsLocation&>()))
            .Times(1);

    tests::TestRequestInterpreter interpreter;
    QString methodName = "getLocation";
    QVariantList paramValues;
    QVariantList paramDatatypes;

    interpreter.execute(mockCaller, methodName, paramValues, paramDatatypes);
}


TEST(RequestInterpreterDeathTest, get_assertsUnknownInterface) {
    InterfaceRegistrar& registrar = InterfaceRegistrar::instance();

    ASSERT_DEATH(registrar.getRequestInterpreter("unknown interface"), "Assertion.*");
}


TEST_F(RequestInterpreterTest, create_createsGpsInterpreter) {
    InterfaceRegistrar& registrar = InterfaceRegistrar::instance();
    registrar.reset();
    registrar.registerRequestInterpreter<vehicle::GpsRequestInterpreter>(gpsInterfaceName);

    QSharedPointer<IRequestInterpreter> gpsInterpreter = registrar.getRequestInterpreter(gpsInterfaceName);

    EXPECT_FALSE(gpsInterpreter.isNull());
}

TEST_F(RequestInterpreterTest, create_multipleCallsReturnSameInterpreter) {
    InterfaceRegistrar& registrar = InterfaceRegistrar::instance();
    registrar.reset();
    registrar.registerRequestInterpreter<vehicle::GpsRequestInterpreter>(gpsInterfaceName);

    QSharedPointer<IRequestInterpreter> gpsInterpreter1 = registrar.getRequestInterpreter(gpsInterfaceName);
    QSharedPointer<IRequestInterpreter> gpsInterpreter2 = registrar.getRequestInterpreter(gpsInterfaceName);

    EXPECT_EQ(gpsInterpreter1, gpsInterpreter2);
}

TEST_F(RequestInterpreterTest, registerUnregister) {
    InterfaceRegistrar& registrar = InterfaceRegistrar::instance();
    registrar.reset();

    // Register the interface twice and check that the interpreter does not change
    registrar.registerRequestInterpreter<vehicle::GpsRequestInterpreter>(gpsInterfaceName);
    QSharedPointer<IRequestInterpreter> gpsInterpreter1 = registrar.getRequestInterpreter(gpsInterfaceName);
    registrar.registerRequestInterpreter<vehicle::GpsRequestInterpreter>(gpsInterfaceName);
    QSharedPointer<IRequestInterpreter> gpsInterpreter2 = registrar.getRequestInterpreter(gpsInterfaceName);
    EXPECT_EQ(gpsInterpreter1, gpsInterpreter2);

    // Unregister once
    registrar.unregisterRequestInterpreter(gpsInterfaceName);
    QSharedPointer<IRequestInterpreter> gpsInterpreter3 = registrar.getRequestInterpreter(gpsInterfaceName);
    EXPECT_EQ(gpsInterpreter1, gpsInterpreter3);

    // Unregister again
    registrar.unregisterRequestInterpreter(gpsInterfaceName);

    // Register the interface - this should create a new request interpreter
    registrar.registerRequestInterpreter<vehicle::GpsRequestInterpreter>(gpsInterfaceName);
    QSharedPointer<IRequestInterpreter> gpsInterpreter4 = registrar.getRequestInterpreter(gpsInterfaceName);
    EXPECT_NE(gpsInterpreter1, gpsInterpreter4);
}

