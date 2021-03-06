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
#ifndef PRETTYPRINT_H_
#define PRETTYPRINT_H_

#include <gtest/gtest.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <iostream>

#include "joynr/JsonSerializer.h"
#include "joynr/JoynrMessage.h"
#include "joynr/RequestStatus.h"
#include "joynr/RequestStatusCode.h"

#define EXPECT_EQ_QSTRING(a, b) EXPECT_EQ(a, b) << "  Actual: " << b.toStdString() << std::endl << "Expected: " << a.toStdString() << std::endl
#define EXPECT_EQ_QBYTEARRAY(a, b) EXPECT_EQ(a, b) << "  Actual: " << b.constData() << std::endl << "Expected: " << a.constData() << std::endl

//void initPretty(void);

namespace joynr {
namespace types {
    class TStruct;
    void PrintTo(const joynr::types::TStruct& value, ::std::ostream* os);
    class GpsLocation;
    void PrintTo(const joynr::types::GpsLocation& value, ::std::ostream* os);
    class Trip;
    void PrintTo(const joynr::types::Trip& value, ::std::ostream* os);
}
}
void PrintTo(const joynr::JoynrMessage& value, ::std::ostream* os);
void PrintTo(const QString& value, ::std::ostream* os);
void PrintTo(const QByteArray& value, ::std::ostream* os);
//void PrintTo(const QObject& value, ::std::ostream* os);
//void PrintTo(const QVariant& value, ::std::ostream* os);
void PrintTo(const joynr::RequestStatusCode& value, ::std::ostream* os);
void PrintTo(const joynr::RequestStatus& value, ::std::ostream* os);

#endif /* PRETTYPRINT_H_ */
