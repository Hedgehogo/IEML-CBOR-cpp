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
	
		explicit FailedParseException(Reason reason);
		
		[[nodiscard]] auto get_description() const -> std::string override;
	
	private:
		Reason reason_;
	};
}
