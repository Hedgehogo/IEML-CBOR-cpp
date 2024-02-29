#include "FailedParseException.hpp"

namespace ieml_cbor {
	FailedParseException::FailedParseException(FailedParseException::Reason reason) : reason_(reason) {
	}
	
	auto FailedParseException::get_description() const -> std::string {
		switch(reason_) {
			case Reason::IncompleteDocument:
				return "The document is not finished, but the input data has run out.";
			case Reason::InvalidDocumentStructure:
				return "IEML assumes a certain data structure, which has been violated.";
			case Reason::ExpectedNodeType:
				return "An integer value was expected as the node type. But the received value has a different type.";
		}
	}
}