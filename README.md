# Specialized Parking Oriented Technology (SPOT)
A group of friends came together to create a digital parking system to improve parking experiences
for parking enforcers and commuters who use parking spaces.

# Project Requierments
We created the following requierments:

  - Accounts for users that:
    - Keep track of Username/password securely
    - Keep track of user's ID number
    - Keep track of user's parking permit (If applicable)
    - Keep track of user's account balance for parking payments
    - Allow users to pay for parking with account balance
    
  - Mobile App
    - Useres can sign-in to the parking spot they arrive at and pay for parking or use thier parking permit
    - Users can view account information
    - Users can view number of available spaces in parking lots live
  
  - Administrator website that should:
    - Povide admins a live view of Parking space status with maps of multiple parking lots
    - Allow admins to add, remove, and edit user accounts, assume parking pass purchasing mechanisms are already in place
    - Have the ability for Admins to view users in each spot and view thier account information
    
  - Cloud based back end that services that:
    - hosts a database of user accounts.
    - hosts the administartor website, and provide live map updates every 5 seconds.
    - Services Mobile App users
      - Communicating with users by retrieving account information
      - Updates database when a user parkes with a permit
      - Updates databse when a user parks using thier account balance (decrease balance)
      - Retrieve individual parking spot status - open, closed (in use), illegal
  # Design
  Following the requierments, we designed our project in the following manor:
  
