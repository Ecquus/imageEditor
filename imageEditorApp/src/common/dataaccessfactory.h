#pragma once

#include <idataaccess.h>

#include <memory>

namespace fact
{
    auto makeDataAccess() -> std::unique_ptr<IDataAccess>;
}
