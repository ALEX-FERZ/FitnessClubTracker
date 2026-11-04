#pragma once
#include <vector>
#include "models/Subscription.h"

namespace SubscriptionRepository {
int create(const Subscription& sub);
std::vector<Subscription> getByClient(int clientId);
void updateVisits(int id, int visitsLeft);
void remove(int id);
}
