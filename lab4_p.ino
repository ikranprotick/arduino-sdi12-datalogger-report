void setup() 
{
  Serial.begin(9600);
  Serial.println("Welcome to Arduino Menu");

}

void loop()
{
  
  Serial.println("\nMain Menu:");
  Serial.println("1. Option 1");
  Serial.println("2. Option 2");
  Serial.println("3. Option 3");
  Serial.println("4. Option 4");
  Serial.println("Choose an option:");

  while (!Serial.available()); 

  int option = Serial.parseInt(); 

  switch (option) 
  {
    case 1:
      submenu1();
      break;
    case 2:
      submenu2();
      break;
    case 3:
      submenu3();
      break;
    case 4:
      submenu4();
      break;
    default:
      Serial.println("Invalid option");
      break;
  }
}


void submenu1() 
{
  Serial.println("\nSubmenu 1:");
  Serial.println("1. Action 1");
  Serial.println("2. Action 2");
  Serial.println("3. Action 3");
  Serial.println("Choose an action:");

  while (!Serial.available()); 

  int action = Serial.parseInt(); 

  switch (action) 
  {
    case 1:
      
      Serial.println("Performing Action 1 for Submenu 1");
      break;
    case 2:
      
      Serial.println("Performing Action 2 for Submenu 1");
      break;
    case 3:
      
      Serial.println("Performing Action 3 for Submenu 1");
      break;
    default:
      Serial.println("Invalid action");
      break;
  }
}

void submenu2() 
{
  Serial.println("\nSubmenu 2:");
  Serial.println("1. Action 1");
  Serial.println("2. Action 2");
  Serial.println("3. Action 3");
  Serial.println("Choose an action:");

  while (!Serial.available()); 

  int action = Serial.parseInt(); 

  switch (action) 
  {
    case 1:
      
      Serial.println("Performing Action 1 for Submenu 2");
      break;
    case 2:
      
      Serial.println("Performing Action 2 for Submenu 2");
      break;
    case 3:
      
      Serial.println("Performing Action 3 for Submenu 2");
      break;
    default:
      Serial.println("Invalid action");
      break;
  }
}

void submenu3() 
{
  Serial.println("\nSubmenu 3:");
  Serial.println("1. Action 1");
  Serial.println("2. Action 2");
  Serial.println("3. Action 3");
  Serial.println("Choose an action:");

  while (!Serial.available()); 

  int action = Serial.parseInt(); 

  switch (action) 
  {
    case 1:
      
      Serial.println("Performing Action 1 for Submenu 3");
      break;
    case 2:
      
      Serial.println("Performing Action 2 for Submenu 3");
      break;
    case 3:
      
      Serial.println("Performing Action 3 for Submenu 3");
      break;
    default:
      Serial.println("Invalid action");
      break;
  }
}

void submenu4() 
{
  Serial.println("\nSubmenu 4:");
  Serial.println("1. Action 1");
  Serial.println("2. Action 2");
  Serial.println("3. Action 3");
  Serial.println("Choose an action:");

  while (!Serial.available()); 

  int action = Serial.parseInt(); 

  switch (action) 
  {
    case 1:
      
      Serial.println("Performing Action 1 for Submenu 4");
      break;
    case 2:
      
      Serial.println("Performing Action 2 for Submenu 4");
      break;
    case 3:
      
      Serial.println("Performing Action 3 for Submenu 4");
      break;
    default:
      Serial.println("Invalid action");
      break;
  }
}
