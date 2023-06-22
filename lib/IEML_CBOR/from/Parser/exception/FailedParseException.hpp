#pragma once

#include "IEML/node.hpp"

namespace ieml_cbor {
	class FailedParseException : public orl::BaseException {
	public:
		enum class Reason {
			IncompleteDocument,
			InvalidDocumentStructure,
			ExpectedNodeType,
			
		};
	
	private:
		Reason reason_;
	
	public:
		FailedParseException(Reason reason);
		
		std::string get_description() const override;
	};
}
