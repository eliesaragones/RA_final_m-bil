#include "mwc_taxibot/taxi_core.hpp"

#include <cassert>
#include <iostream>

int main()
{
    mwc_taxibot::TaxiCore taxi;
    taxi.addZone({"base", 0.0, 0.0, 0.0, "BASE"});
    taxi.addZone({"entrada", 1.5, 0.0, 0.0, "ENTRADA"});
    taxi.addZone({"auditorio", 4.0, 2.0, 1.57, "AUDITORIO"});

    assert(taxi.state() == mwc_taxibot::TaxiState::IDLE);
    assert(taxi.hasZone("entrada"));
    assert(!taxi.hasZone("cafeteria"));

    const auto bad_zone = taxi.requestTaxi("entrada", "cafeteria");
    assert(!bad_zone.accepted);
    assert(bad_zone.message == "Destino no encontrado: cafeteria");
    assert(taxi.state() == mwc_taxibot::TaxiState::IDLE);

    const auto same_zone = taxi.requestTaxi("entrada", "entrada");
    assert(!same_zone.accepted);
    assert(same_zone.message == "El punto de recogida y destino no pueden ser iguales");

    const auto accepted = taxi.requestTaxi("entrada", "auditorio");
    assert(accepted.accepted);
    assert(taxi.state() == mwc_taxibot::TaxiState::GOING_TO_PICKUP);
    assert(taxi.pickupZone() == "entrada");
    assert(taxi.destinationZone() == "auditorio");
    assert(taxi.busy());
    assert(taxi.currentGoal().name == "entrada");

    const auto occupied = taxi.requestTaxi("base", "auditorio");
    assert(!occupied.accepted);
    assert(occupied.message == "Taxi ocupado");

    taxi.markNavigationSucceeded();
    assert(taxi.state() == mwc_taxibot::TaxiState::WAITING_PASSENGER);

    taxi.markPassengerWaitComplete();
    assert(taxi.state() == mwc_taxibot::TaxiState::GOING_TO_DESTINATION);
    assert(taxi.currentGoal().name == "auditorio");

    taxi.markNavigationSucceeded();
    assert(taxi.state() == mwc_taxibot::TaxiState::CONFIRMING_ZONE);

    taxi.markZoneConfirmed(true);
    assert(taxi.state() == mwc_taxibot::TaxiState::RETURNING_TO_BASE);
    assert(taxi.currentGoal().name == "base");

    taxi.markNavigationSucceeded();
    assert(taxi.state() == mwc_taxibot::TaxiState::IDLE);
    assert(!taxi.busy());

    taxi.requestTaxi("entrada", "auditorio");
    taxi.markVisualObstacle(true);
    assert(taxi.state() == mwc_taxibot::TaxiState::OBSTACLE_DETECTED);
    taxi.markVisualObstacle(false);
    assert(taxi.state() == mwc_taxibot::TaxiState::GOING_TO_PICKUP);

    std::cout << "taxi_core_test passed\n";
    return 0;
}
