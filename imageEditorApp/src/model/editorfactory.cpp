#include <editorfactory.h>
#include "editor.h"

auto fact::makeEditor(IEditor::InterpMethod interpMethod, bool debug) -> std::unique_ptr<IEditor>
{
    return std::make_unique<Editor>(interpMethod, debug);
}
