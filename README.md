
# BRIQ Algae PBR
Algae photobioreactor(PBR) platform - Focused on algae as potential food sources or means for small scale carbon sequestration, apposed to biofuel, the project hopes to provide the starting point for collaboration, to bring PBR technology out of the lab and into our everyday lives. 

*WORK IN PROGRESS*
<p align="center">
  <img src="https://github.com/mishave/vhausAlgae/blob/master/vhausAlageMechanical/Misc/systemOverview.PNG" />
</p>

## Design Overview :v:
### Cultivation
- Close loop design
- 5.8L capacity
- Sensing: 
  - pH
  - Water Temperature
  - Dissolved Oxygen
  - Lux
  - Water Level
  - External Temperature & Humidity
- Control:
  - pH up/Down
  - Culture Temperature
  - Light Cycle & Intensity


### Harvesting
- 15L capacity
- crossflow filter

### Software
- Home Assistant
  - Mosquitto broker - MQTT
  - AppDaemon - Python
- Google Sheets API

## Next Steps :point_up:
- [ ] Transfer drawings from Eagle to KiCad
- [ ] Add heating to tempreture control
- [ ] Remove Arduino Mega
