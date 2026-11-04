#include "database/Database.h"
#include "repositories/Repositories.h"

namespace DictionaryRepository {

std::vector<RoleInfo> getRoles() {
    Statement st(Database::instance().handle(), "SELECT role_id, name FROM roles ORDER BY role_id");
    std::vector<RoleInfo> result;
    while (st.step()) result.push_back({st.columnInt(0), st.columnText(1)});
    return result;
}

std::vector<BookingStatusInfo> getBookingStatuses() {
    Statement st(Database::instance().handle(),
                 "SELECT status_id, name FROM booking_statuses ORDER BY status_id");
    std::vector<BookingStatusInfo> result;
    while (st.step()) result.push_back({st.columnInt(0), st.columnText(1)});
    return result;
}

}  // namespace DictionaryRepository
