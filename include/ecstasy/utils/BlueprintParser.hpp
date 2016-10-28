#pragma once
/*******************************************************************************
 * Copyright 2011 See AUTHORS file.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#include <memory>
#include <string>
#include <istream>

namespace ecstasy {
	class EntityBlueprint;
	/**
	 * Parse a blueprint file.
	 *
	 * @param filename the file to parse
	 * @param result an empty shared_ptr to store the result.
	 * @return An empty string on success, otherwise an error message containing line information.
	 */
	std::string parseBlueprint(const std::string& filename, std::shared_ptr<EntityBlueprint>& result);

	/**
	 * Parse a blueprint file from a stream.
	 *
	 * @param stream the stream to read from
	 * @param result an empty shared_ptr to store the result.
	 * @return An empty string on success, otherwise an error message containing line information.
	 */
	std::string parseBlueprint(std::istream& stream, std::shared_ptr<EntityBlueprint>& result);
}
