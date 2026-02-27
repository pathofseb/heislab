  def main():
      """Main elevator control loop"""
      current_floor = find_position_on_boot()
      direction = STOP

      while True:
          # Safety first
          if stop_button_pressed():
              emergency_stop()
              continue

          # Process inputs
          poll_all_buttons()

          # Decision logic
          if has_any_orders():
              next_floor = get_next_floor(current_floor, direction)
              set_motor_direction(direction)

              if at_floor(current_floor) and has_order(current_floor):
                  stop_motor()
                  open_door(current_floor)
                  clear_floor_orders(current_floor)
                  wait_with_obstruction_check(3.0)
                  close_door()
          else:
              set_motor_direction(STOP)