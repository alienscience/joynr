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
#ifndef REQUEST_H
#define REQUEST_H

#include "joynr/JoynrCommonExport.h"

#include <QObject>
#include <QVariant>
#include <QSharedPointer>

namespace joynr {

class JOYNRCOMMON_EXPORT Request : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString requestReplyId READ getRequestReplyId WRITE setRequestReplyId)
    Q_PROPERTY(QString methodName READ getMethodName WRITE setMethodName)
    Q_PROPERTY(QList<QVariant> params READ getParams WRITE setParams)
    Q_PROPERTY(QList<QVariant> paramDatatypes READ getParamDatatypes WRITE setParamDatatypes)

public:
    Request();

    Request(const Request& other);
    Request& operator=(const Request& other);
    bool operator==(const Request& other) const;

    const QString& getRequestReplyId() const;
    void setRequestReplyId(const QString& requestReplyId);

    const QString& getMethodName() const;
    void setMethodName(const QString& methodName);

    QList<QVariant> getParams() const;
    void setParams(const QList<QVariant>& params);

    void addParam(QVariant value, QString datatype);

    QList<QVariant> getParamDatatypes() const;
    void setParamDatatypes(const QList<QVariant>& paramDatatypes);

private:
    static QVariant parameterType(const QVariant& param);

    QString requestReplyId;
    QString methodName;
    QList<QVariant> params;
    QList<QVariant> paramDatatypes;
};


} // namespace joynr

Q_DECLARE_METATYPE(joynr::Request)
Q_DECLARE_METATYPE(QSharedPointer<joynr::Request>)
#endif //REQUEST_H
