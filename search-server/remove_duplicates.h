#pragma once

#include "document.h"
#include "search_server.h"
#include "log_duration.h"

#include <set>
#include <vector>
#include <string>
#include <stack>

void RemoveDuplicates(SearchServer& search_server);