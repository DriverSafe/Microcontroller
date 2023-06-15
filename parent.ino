// Ultrasonic sensor
const int trigPin = D1;
const int echoPin = D8;

// Motor
#define enA D2
#define inA1 D3
#define inA2 D4

#define enB D7
#define inB1 D5
#define inB2 D6

float speedKM;
int value;
long duration;

int motorSpeed = 0;  // Current motor speed
int targetSpeed = 0; // Target motor speed

// For detect break or accident
#define MAX_SIZE 3
int distance_queue[MAX_SIZE];
bool accidentHappened = false;

void setup()
{
    Serial.begin(115200);

    // Motor
    value = 255;
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

    pinMode(enA, OUTPUT); // Set motor control pins as outputs
    pinMode(inA1, OUTPUT);
    pinMode(inA2, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(inB1, OUTPUT);
    pinMode(inB2, OUTPUT);

    analogWrite(enA, 0); // Initialize motor speed to 0
    analogWrite(enB, 0);

    // Ultrasonic
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

int locationRequest = 0;
void loop()
{
    locationRequest++;

    if (locationRequest == 20000)
    {
        Serial.println("request_send_location");
        locationRequest = 0;
    }

    // Use Ultrasonic sensor to calculate the distance
    int distance = getDistance();
    enqueue(distance);
    Serial.println(distance);

    // Motor
    // Adjust motor speed based on distance
    if (distance < 6)
    {
        crash();
    }
    else if (distance < 10)
    {
        if (value > 122)
        {
            for (value = value; value >= 0; value--)
            {
                Serial.println("request_status_break");
                moveMotorsForward(value);
                delay(1);
            }
            Serial.println("request_status_stopped");
        }
        else
        {
            moveMotorsForward(value); // Keep the current value
        }
    }
    else if (distance < 20)
    {
        Serial.println("request_status_break");
        if (value > 220)
        {
            for (value = value; value >= 193; value--)
            {
                moveMotorsForward(value);
                delay(10);
            }
        }
        else
        {
            moveMotorsForward(value); // Keep the current value
        }
    }
    else if (distance < 30)
    {
        Serial.println("request_status_break");
        if (value == 255)
        {
            for (value = 255; value >= 225; value--)
            {
                moveMotorsForward(value);
                delay(10);
            }
        }
        else
        {
            moveMotorsForward(value); // Keep the current value
        }
    }
    else
    {
        value = 255;
        moveMotorsForward(value);
        Serial.println("request_status_driving");
    }
}

void moveMotorsForward(int speed)
{
    digitalWrite(inA1, LOW);
    digitalWrite(inA2, HIGH);
    digitalWrite(inB1, HIGH);
    digitalWrite(inB2, LOW);

    analogWrite(enA, speed); // Set motor speed
    analogWrite(enB, speed);
    speedKM = (speed - 120) * (50 - 0) / (255 - 120);
    Serial.print(speedKM);
    Serial.println(" KM/h");
}

void stopMotors()
{
    digitalWrite(inA1, LOW);
    digitalWrite(inA2, LOW);
    digitalWrite(inB1, LOW);
    digitalWrite(inB2, LOW);

    analogWrite(enA, 0); // Set motor speed to 0
    analogWrite(enB, 0);
}

void crash()
{
    accidentHappened = true;
    Serial.println("request_status_accident");
    Serial.println("request_accident");
}

int getDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    int distance = duration * 0.034 / 2;

    return distance;
}

void enqueue(int element)
{
    for (int i = MAX_SIZE - 1; i > 0; i--)
    {
        distance_queue[i] = distance_queue[i - 1];
    }

    distance_queue[0] = element;
}
