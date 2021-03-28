#include <dataaccessfactory.h>
#include "dataaccess.h"

auto fact::makeDataAccess() -> std::unique_ptr<IDataAccess>
{
    return std::make_unique<DataAccess>();
}
