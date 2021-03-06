package io.joynr.generator.cpp.communicationmodel
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
import org.franca.core.franca.FEnumerationType
import org.franca.core.franca.FCompoundType
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions

class CommunicationModelGenerator {
	
	@Inject
	private extension JoynrCppGeneratorExtensions
	
	@Inject
	InterfaceHTemplate interfaceH;
	
	@Inject
	InterfaceCppTemplate interfaceCpp;

	@Inject
	EnumHTemplate enumh;
	
	@Inject
	TypeHTemplate typeH;

	@Inject
	TypeCppTemplate typeCpp;
	
	def doGenerate(FModel fModel, 
		IFileSystemAccess sourceFileSystem, 
		IFileSystemAccess headerFileSystem, 
		String sourceContainerPath,
		String headerContainerPath
	){
		val dataTypePath = sourceContainerPath + "datatypes" + File::separator
		val headerDataTypePath = 
			if (sourceFileSystem == headerFileSystem) 
				headerContainerPath + "datatypes" + File::separator
			else
				headerContainerPath
		
		for( type: getComplexDataTypes(fModel)){
			if(type instanceof FCompoundType) {
				val sourcepath = dataTypePath + getPackageSourceDirectory(type) + File::separator
				val headerpath = headerDataTypePath + getPackagePathWithJoynrPrefix(type, File::separator) + File::separator
				
				headerFileSystem.generateFile(
					headerpath + type.joynrName + ".h",
					typeH.generate(type).toString
				)
	
				sourceFileSystem.generateFile(
					sourcepath + type.joynrName + ".cpp",
					typeCpp.generate(type).toString
				)
			}
		}
		
		for( type: getEnumDataTypes(fModel)){
			val headerpath = headerDataTypePath + getPackagePathWithJoynrPrefix(type, File::separator) + File::separator
			
			headerFileSystem.generateFile(
				headerpath + type.joynrName + ".h",
				enumh.generate(type as FEnumerationType).toString
			)
		}

		val interfacePath = sourceContainerPath + "interfaces" + File::separator
		val headerInterfacePath = 
			if (sourceFileSystem == headerFileSystem) 
				headerContainerPath + "interfaces" + File::separator
			else
				headerContainerPath
		
		for(serviceInterface: fModel.interfaces){
			val sourcepath = interfacePath + getPackageSourceDirectory(serviceInterface) + File::separator 
			val headerpath = headerInterfacePath + getPackagePathWithJoynrPrefix(serviceInterface, File::separator) + File::separator 

			headerFileSystem.generateFile(
				headerpath + "I" + serviceInterface.joynrName + ".h",
				interfaceH.generate(serviceInterface).toString
			);
			
			sourceFileSystem.generateFile(
				sourcepath + "I" + serviceInterface.joynrName + ".cpp",
				interfaceCpp.generate(serviceInterface).toString
			);
		}
		
	}
}