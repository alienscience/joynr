package io.joynr.integration.matchers;

/*
 * #%L
 * joynr::java::messaging::service-common
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

import java.util.List;

import org.hamcrest.BaseMatcher;
import org.hamcrest.Description;
import org.hamcrest.Matcher;

import com.jayway.restassured.path.json.JsonPath;

public class MessagingServiceResponseMatchers {

    public static Matcher<List<String>> containsMessage(final String msgId) {

        return new BaseMatcher<List<String>>() {

            @Override
            public boolean matches(Object item) {

                @SuppressWarnings("unchecked")
                List<String> messages = (List<String>) item;

                for (String message : messages) {
                    JsonPath jsonMessage = new JsonPath(message);
                    String msgIdInJson = jsonMessage.getString("header.msgId");

                    if (msgIdInJson != null && msgIdInJson.equals(msgId)) {
                        return true;
                    }
                    ;
                }

                return false;
            }

            @Override
            public void describeTo(Description description) {
                description.appendText("contains message ID '" + msgId + "'");
            }

        };
    }
}
