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
#include "utils/TestQString.h"
#include "joynr/Util.h"
#include <QString>
#include <QByteArray>
#include <QList>

using namespace joynr;

TEST(UtilTest, splitIntoJsonObjects)
{
    QByteArray inputStream;
    QList<QByteArray> result;

    inputStream = " not a valid Json ";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(0, result.size());

    inputStream = "{\"id\":34}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(1, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"id\":34}") );

    inputStream = "{\"message\":{one:two}}{\"id\":35}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"message\":{one:two}}") );
    EXPECT_QSTREQ(QString::fromUtf8(result.at(1)),
                  QString("{\"id\":35}") );

    //payload may not contain { or } outside a string.
    inputStream = "{\"id\":3{4}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(0, result.size());

    //  { within a string should be ok
    inputStream = "{\"messa{ge\":{one:two}}{\"id\":35}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"messa{ge\":{one:two}}") );

    //  } within a string should be ok
    inputStream = "{\"messa}ge\":{one:two}}{\"id\":35}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"messa}ge\":{one:two}}") );

    //  }{ within a string should be ok
    inputStream = "{\"messa}{ge\":{one:two}}{\"id\":35}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"messa}{ge\":{one:two}}") );

    //  {} within a string should be ok
    inputStream = "{\"messa{}ge\":{one:two}}{\"id\":35}";
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"messa{}ge\":{one:two}}") );

    //string may contain \"
    inputStream = "{\"mes\\\"sa{ge\":{one:two}}{\"id\":35}";
    //inputStream:{"mes\"sa{ge":{one:two}}{"id":35}
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"mes\\\"sa{ge\":{one:two}}") );


    inputStream = "{\"mes\\\\\"sa{ge\":{one:two}}{\"id\":35}";
    // inputStream: {"mes\\"sa{ge":{one:two}}{"id":35}
    // / does not escape within JSON String, so the string should not be ended after mes\"
    result = Util::splitIntoJsonObjects(inputStream);
    EXPECT_EQ(2, result.size());
    EXPECT_QSTREQ(QString::fromUtf8(result.at(0)),
                  QString("{\"mes\\\\\"sa{ge\":{one:two}}"));
}

TEST(UtilTest, convertListToQVariantList){

    QList<int> intlist;
    QList<QVariant> qvarList;

    intlist.append(2);
    intlist.append(5);
    intlist.append(-1);

    qvarList.append(QVariant(2));
    qvarList.append(QVariant(5));
    qvarList.append(QVariant(-1));

    QList<QVariant> convertedQvarList = Util::convertListToVariantList<int>(intlist);
    QList<int> convertedIntList = Util::convertVariantListToList<int>(qvarList);

    EXPECT_EQ(convertedQvarList, qvarList);
    EXPECT_EQ(convertedIntList, intlist);

    QList<QVariant> reconvertedQvarList = Util::convertListToVariantList<int>(convertedIntList);
    QList<int> reconvertedIntList = Util::convertVariantListToList<int>(convertedQvarList);

    EXPECT_EQ(reconvertedQvarList, qvarList);
    EXPECT_EQ(reconvertedIntList, intlist);


}
