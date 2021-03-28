#pragma once

#include <ieditor.h>

#include <memory>

namespace fact
{
    auto makeEditor(IEditor::InterpMethod interpMethod, bool debug) -> std::unique_ptr<IEditor>;
}
