#pragma once

#include <ch_stl/hash.h>
#include <ch_stl/time.h>

using Entity_Id = u64;

CH_FORCEINLINE u64 hash(Entity_Id e_id) {
	return ch::fnv1_hash(&e_id, sizeof(e_id));
}

CH_FORCEINLINE Entity_Id get_unique_id() {
	f64 current_time = ch::get_time_in_seconds();
	return (Entity_Id)(current_time * 1000000000.0);
}