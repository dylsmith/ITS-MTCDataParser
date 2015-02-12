
% of people who can potentially share with at least one other person
% of people who actually share
% of trips that are doable
% of trips that are shreable
% of tours actually shareable

When looking at tour level, each leg of the trip, if doable, has a random chance of failure

Make sas many possible people share during sharing selection
Then determine sharing effectiveness with minimum and maximujm people in each car

Record nnumber of shared trips originating/arriving at each point

for VMT, ensure calculated trips were previously driving and not biking
Ensure one person ahs a vehicle in each rideshare

trip potentially shareable requirements:
	distance traveled > some value
	trip mode in some set of viable trip modes
	trip purpose in some set of viable trip purposes
	income < some max income 
	passes random sampling (static % success) //0.1

	globals:
		MaxIncome
		MinDistanceTraveled
		RandomFailChance
		TripModes
		TripPurposes

trip sharing requirements:
	have close origin and destination
	have same depart hour
	have different people
		

tour requirements:
	number of stops < some number
	every trip must be either shared or have a mode in some set of trip modes
	% of actually shared rides must be > some number

					
ride sharing choosing:
	number of riders must be > some number and < some number

	calculate vehicle-mile reduction




take in files like normal, output modified versions based off our data structures at the end
	to output modified version, every variable needs to be read in from the source file and written out to the new one1
		updated person file should not contain people who are ride sharing

		upon starting, merge the already-sharing people with the newly generated non-sharing
