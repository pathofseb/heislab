# Elevator Project Criteria Checklist

## Startup
- [x] Determine current position at startup
- [x] Move to a defined floor if starting between floors
- [ ] Ignore all orders until position is known
- [x] Assume valid physical floor limits

## Orders
- [ ] All orders are eventually served
- [ ] Serve floors in current travel direction
- [ ] Do not serve hall calls in opposite direction while moving
- [ ] Clear all orders at a floor when stopping there
- [ ] Stay idle when no pending orders

## Signals and Lights
- [ ] Order light stays on until order is served
- [ ] Order light turns off when order is cleared
- [ ] Correct floor indicator is shown at a floor
- [ ] While between floors, show last passed floor
- [ ] Only one floor indicator light is active at a time
- [x] Stop button light is on while pressed

## Door
- [ ] Door opens for 3 seconds at ordered floor
- [ ] Door closes after 3 seconds
- [ ] Elevator does not move while door is open
- [ ] If stop is pressed at a floor, door stays open while pressed and 3 seconds after release
- [ ] Obstruction keeps door open while active
- [ ] After obstruction clears, door closes after 3 seconds

## Safety
- [ ] Door never opens outside a floor
- [ ] Elevator never moves outside valid floor range
- [ ] Elevator stops immediately when stop button is pressed
- [ ] All active orders are cleared when stop is pressed
- [ ] New orders are ignored while stop is active
- [x] After stop is released, elevator remains idle until a new order

## Robustness
- [ ] Obstruction has no effect when door is closed
- [ ] Program does not crash during normal operation
- [ ] No restart required during testing
- [x] After calibration, elevator always knows its position
- [ ] No unrealistic elevator behavior