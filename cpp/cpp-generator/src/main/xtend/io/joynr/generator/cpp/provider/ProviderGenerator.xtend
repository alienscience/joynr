package io.joynr.generator.cpp.provider
/*
 * !!!
 *
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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

import com.google.inject.Inject
import java.io.File
import org.eclipse.xtext.generator.IFileSystemAccess
import org.franca.core.franca.FModel
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions

class ProviderGenerator {
		
	@Inject
	private extension JoynrCppGeneratorExtensions

	@Inject
	InterfaceRequestCallerHTemplate interfaceRequestCallerH;

	@Inject
	InterfaceRequestCallerCppTemplate interfaceRequestCallerCpp;
	
	@Inject
	InterfaceRequestInterpreterHTemplate interfaceRequestInterpreterH;

	@Inject
	InterfaceRequestInterpreterCppTemplate interfaceRequestInterpreterCpp;

	@Inject
	InterfaceProviderCppTemplate interfaceProviderCppTemplate;

	@Inject
	InterfaceProviderHTemplate interfaceProviderHTemplate;
	
	def doGenerate(
		FModel model, 
		IFileSystemAccess sourceFileSystem, 
		IFileSystemAccess headerFileSystem, 
		String sourceContainerPath,
		String headerContainerPath
	){

		for(serviceInterface: model.interfaces){
			val sourcePath = sourceContainerPath + getPackageSourceDirectory(serviceInterface) + File::separator;
			val headerPath = headerContainerPath + getPackagePathWithJoynrPrefix(serviceInterface, File::separator) + File::separator;
			var serviceName = serviceInterface.joynrName
				
			headerFileSystem.generateFile(
				headerPath + serviceName + "RequestInterpreter.h",
				interfaceRequestInterpreterH.generate(serviceInterface).toString
			);

			sourceFileSystem.generateFile (
				sourcePath + serviceName + "RequestInterpreter.cpp",
				interfaceRequestInterpreterCpp.generate(serviceInterface).toString
			);
			
			headerFileSystem.generateFile(
				headerPath + serviceName + "RequestCaller.h",
				interfaceRequestCallerH.generate(serviceInterface).toString
			);

			sourceFileSystem.generateFile(
				sourcePath + serviceName + "RequestCaller.cpp",
				interfaceRequestCallerCpp.generate(serviceInterface).toString
			);
			
			sourceFileSystem.generateFile(
				sourcePath + serviceName + "Provider.cpp",
				interfaceProviderCppTemplate.generate(serviceInterface).toString
			);
			
			headerFileSystem.generateFile(
				headerPath + serviceName + "Provider.h",
				interfaceProviderHTemplate.generate(serviceInterface).toString
			);
		}		
	}
}
