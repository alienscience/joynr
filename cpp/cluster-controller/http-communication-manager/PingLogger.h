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
#ifndef PINGLOGGER_H
#define PINGLOGGER_H

#include "joynr/joynrlogging.h"

#include <QObject>
#include <QProcess>

namespace joynr {

class PingLogger :  public QObject {
    Q_OBJECT
public:
    PingLogger(const QString& host, QObject* parent = 0);
    virtual ~PingLogger();

signals:

public slots:
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void readyReadStandardError();
    void readyReadStandardOutput();
    void started();
    void stateChanged(QProcess::ProcessState newState);

private:
    void readAndLogAvailableInput();

    static joynr_logging::Logger* logger;
    QProcess ping;
};


} // namespace joynr
#endif//PINGLOGGER_H
