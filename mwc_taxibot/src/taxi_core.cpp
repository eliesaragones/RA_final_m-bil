#include "mwc_taxibot/taxi_core.hpp"

namespace mwc_taxibot
{

std::string toString(const TaxiState state)
{
    switch (state)
    {
        case TaxiState::IDLE:
            return "IDLE";
        case TaxiState::GOING_TO_PICKUP:
            return "GOING_TO_PICKUP";
        case TaxiState::WAITING_PASSENGER:
            return "WAITING_PASSENGER";
        case TaxiState::GOING_TO_DESTINATION:
            return "GOING_TO_DESTINATION";
        case TaxiState::CONFIRMING_ZONE:
            return "CONFIRMING_ZONE";
        case TaxiState::OBSTACLE_DETECTED:
            return "OBSTACLE_DETECTED";
        case TaxiState::RETURNING_TO_BASE:
            return "RETURNING_TO_BASE";
        case TaxiState::ERROR:
            return "ERROR";
    }
    return "ERROR";
}

TaxiCore::TaxiCore()
    : state_(TaxiState::IDLE),
      state_before_obstacle_(TaxiState::IDLE),
      base_zone_("base"),
      return_to_base_after_service_(true)
{
    status_message_ = "Taxi disponible";
}

void TaxiCore::addZone(const Zone& zone)
{
    if (zone.name.empty())
    {
        throw std::invalid_argument("La zona no puede tener nombre vacío");
    }
    zones_[zone.name] = zone;
}

bool TaxiCore::hasZone(const std::string& name) const
{
    return zones_.find(name) != zones_.end();
}

const Zone& TaxiCore::zone(const std::string& name) const
{
    const auto it = zones_.find(name);
    if (it == zones_.end())
    {
        throw std::out_of_range("Zona no encontrada: " + name);
    }
    return it->second;
}

void TaxiCore::setBaseZone(const std::string& name)
{
    base_zone_ = name;
}

void TaxiCore::setReturnToBaseAfterService(const bool enabled)
{
    return_to_base_after_service_ = enabled;
}

TaxiRequestResult TaxiCore::requestTaxi(const std::string& pickup_zone, const std::string& destination_zone)
{
    if (state_ != TaxiState::IDLE)
    {
        return {false, "Taxi ocupado"};
    }
    if (!hasZone(pickup_zone))
    {
        return {false, "Punto de recogida no encontrado: " + pickup_zone};
    }
    if (!hasZone(destination_zone))
    {
        return {false, "Destino no encontrado: " + destination_zone};
    }
    if (pickup_zone == destination_zone)
    {
        return {false, "El punto de recogida y destino no pueden ser iguales"};
    }

    pickup_zone_ = pickup_zone;
    destination_zone_ = destination_zone;
    setGoal(pickup_zone_);
    state_ = TaxiState::GOING_TO_PICKUP;
    status_message_ = "Solicitud aceptada: " + pickup_zone_ + " -> " + destination_zone_;
    error_message_.clear();
    return {true, status_message_};
}

void TaxiCore::markNavigationSucceeded()
{
    switch (state_)
    {
        case TaxiState::GOING_TO_PICKUP:
            state_ = TaxiState::WAITING_PASSENGER;
            current_goal_.clear();
            status_message_ = "Pasajero esperando en " + pickup_zone_;
            break;
        case TaxiState::GOING_TO_DESTINATION:
            state_ = TaxiState::CONFIRMING_ZONE;
            current_goal_.clear();
            status_message_ = "Destino alcanzado, confirmando zona " + destination_zone_;
            break;
        case TaxiState::RETURNING_TO_BASE:
            finishService();
            break;
        default:
            break;
    }
}

void TaxiCore::markNavigationFailed(const std::string& reason)
{
    state_ = TaxiState::ERROR;
    current_goal_.clear();
    error_message_ = reason.empty() ? "Fallo de navegación" : reason;
    status_message_ = error_message_;
}

void TaxiCore::markPassengerWaitComplete()
{
    if (state_ != TaxiState::WAITING_PASSENGER)
    {
        return;
    }
    setGoal(destination_zone_);
    state_ = TaxiState::GOING_TO_DESTINATION;
    status_message_ = "Pasajero recogido. Navegando hacia " + destination_zone_;
}

void TaxiCore::markZoneConfirmed(const bool confirmed)
{
    if (state_ != TaxiState::CONFIRMING_ZONE)
    {
        return;
    }
    if (!confirmed)
    {
        markNavigationFailed("No se pudo confirmar visualmente la zona " + destination_zone_);
        return;
    }
    status_message_ = "Servicio completado en " + destination_zone_;
    if (return_to_base_after_service_ && hasZone(base_zone_))
    {
        setGoal(base_zone_);
        state_ = TaxiState::RETURNING_TO_BASE;
        status_message_ += ". Volviendo a base";
    }
    else
    {
        finishService();
    }
}

void TaxiCore::markVisualObstacle(const bool detected)
{
    if (detected)
    {
        if (state_ == TaxiState::GOING_TO_PICKUP || state_ == TaxiState::GOING_TO_DESTINATION)
        {
            state_before_obstacle_ = state_;
            state_ = TaxiState::OBSTACLE_DETECTED;
            status_message_ = "Obstáculo visual detectado";
        }
        return;
    }

    if (state_ == TaxiState::OBSTACLE_DETECTED)
    {
        state_ = state_before_obstacle_;
        status_message_ = "Obstáculo despejado. Continuando servicio";
    }
}

void TaxiCore::resetToIdle()
{
    finishService();
}

TaxiState TaxiCore::state() const
{
    return state_;
}

TaxiState TaxiCore::stateBeforeObstacle() const
{
    return state_before_obstacle_;
}

bool TaxiCore::busy() const
{
    return state_ != TaxiState::IDLE;
}

bool TaxiCore::hasActiveGoal() const
{
    return !current_goal_.empty();
}

const Zone& TaxiCore::currentGoal() const
{
    return zone(current_goal_);
}

std::string TaxiCore::pickupZone() const
{
    return pickup_zone_;
}

std::string TaxiCore::destinationZone() const
{
    return destination_zone_;
}

std::string TaxiCore::statusMessage() const
{
    return status_message_;
}

std::string TaxiCore::errorMessage() const
{
    return error_message_;
}

void TaxiCore::setGoal(const std::string& zone_name)
{
    if (!hasZone(zone_name))
    {
        throw std::out_of_range("Objetivo no encontrado: " + zone_name);
    }
    current_goal_ = zone_name;
}

void TaxiCore::finishService()
{
    state_ = TaxiState::IDLE;
    state_before_obstacle_ = TaxiState::IDLE;
    pickup_zone_.clear();
    destination_zone_.clear();
    current_goal_.clear();
    error_message_.clear();
    status_message_ = "Taxi disponible";
}

}  // namespace mwc_taxibot
