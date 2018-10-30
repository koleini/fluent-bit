--[[

   This Lua script returns a moving average over the records of schema {event: {measurement: :value}} received form MQTT.

   A sample configuration:

   [SERVICE]
     Flush  1

   [INPUT]
     Name   mqtt
     Tag    data
     Listen 0.0.0.0
     Port   1883

   [FILTER]
     Name    lua
     Match   *
     script  moving_average.lua
     call    cb_print
     window  30

   [OUTPUT]
     Name   stdout
     Match  *
]]

average = {reading = 0}

-- Print record to the standard output
function cb_print(tag, timestamp, record)
  size = table.getn(_window)
  if size == _window_info.size then
    average.reading = (average.reading * size - _window[_window_info.head].record.event.measurement + record.event.measurement)/size
  else
    average.reading = (average.reading * size + record.event.measurement)/(size + 1)
  end

  return 1, timestamp, average
end
