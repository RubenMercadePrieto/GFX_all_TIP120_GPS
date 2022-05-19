//for C++ vectors!
using namespace std;

int markSize = 5;
uint16_t graphColor = WHITE;
uint16_t graphColor2 = YELLOW;
uint16_t BackgroundColor = BLACK;
bool pointLine = true;
uint16_t pointLineColor = WHITE;
bool pointLines = true;
uint16_t lineColor = BLUE;
uint16_t lineColor2 = RED;

String graphName = "DHT data";

// reasonable limits, will be find out from the data
float Tmin = 25.0;
float Tmax = 25.0;
float Hmin = 50;
float Hmax = 50;

// Plot dimensions
const int originX = 45;
const int originY = 200;
const int sizeX = 250;
const int sizeY = 170;
const int minorSizeY = (originY + 7);
const int minorSizeX = (originX - 7);

//X and Y axis - only show numXticks number of points
const int numYticks = 7;
const int numXticks = 30;  //important adjustable parameter
float valTempAxis[numYticks];
float valHumAxis[numYticks];
float mark[numXticks];


float Data[numXticks][2];   // temp, hum
String DataTimeStr[numXticks][2];   // Date, time
int locTemp[numXticks];       //vector with the poisitions of the data shown in graph
int locHum[numXticks];
int numberSizeY = (sizeY / (numYticks - 1));

void drawEmptyGraph(int Xpoints)
{
  gfx->fillScreen(BackgroundColor);
  for (int a = 0; a < numYticks; a++) {
    valTempAxis[a] = Tmax - (Tmax - Tmin) / (numYticks - 1) * a;
    valHumAxis[a] = Hmax - (Hmax - Hmin) / (numYticks - 1) * a;
  }
  for (int a = 0; a < Xpoints; a++) { //start from right to left
    mark[a] = -3 + originX + sizeX - sizeX / (Xpoints - 1) * a; //move bit to the right so points dont fall on axis, otherwise hard to remove
    //    Serial.print(mark[a]); Serial.print(",");
    //    PrintCharTFT(String(mark[a]), mark[a], 100, WHITE, BackgroundColor, 1);
  }

  // draw title
  gfx->setCursor(180, 10); // set the cursor
  gfx->setTextColor(graphColor); // set the colour of the text
  gfx->setTextSize(2); // set the size of the text
  gfx->println(graphName);

  // draw plot outline
  gfx->drawRect(originX, originY - sizeY, sizeX, sizeY, graphColor);

  // draw X lables
  for (int i = 0; i < Xpoints; i++)   {
    gfx->drawLine(mark[i], originY, mark[i], minorSizeY, graphColor);
  }
  for (int i = 0; i < numYticks; i++)   { // draw Y axis
    gfx->drawLine(originX, (originY - sizeY + i * numberSizeY), minorSizeX, (originY - sizeY + i * numberSizeY), graphColor); //draw Y axis tick
    PrintCharTFT(String(valTempAxis[i], 1), (minorSizeX - 30), (originY - sizeY - 5 + i * numberSizeY), lineColor, BackgroundColor, 1); //draw Y Temp axis numbers
    PrintCharTFT(String(valHumAxis[i], 1), (minorSizeX - 30), (originY - sizeY + 5 + i * numberSizeY), lineColor2, BackgroundColor, 1); //draw Y Hum axis numbers
  }
}

//needed to map decimal values
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{ return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;}


void drawPlotFile(int NumPoints) {

  drawEmptyGraph(NumPoints);

  //print the legend
  PrintCharTFT("Temperature (C)", originX + 10, originY - sizeY - 10, lineColor, BackgroundColor, 1);
  PrintCharTFT("Humidity (%)", originX + 10, originY - sizeY - 20, lineColor2, BackgroundColor, 1);

  for (int i = 0; i < NumPoints; i++) {
    //map data to limits
    locTemp[i] = mapfloat(Data[i][0], Tmin, Tmax, originY, (originY - sizeY));
    locHum[i] = mapfloat(Data[i][1], Hmin, Hmax, originY, (originY - sizeY));
    //draw points
    gfx->fillRect((mark[i]) - 2, locTemp[i] - 2, markSize, markSize, lineColor); // -2 is to offset point size
    gfx->fillRect((mark[i]) - 2, locHum[i] - 2, markSize, markSize, lineColor2);
    if (pointLine == true) { //draw point lines if true
      gfx->drawRect(mark[i] - 2, locTemp[i] - 2, markSize, markSize, pointLineColor);
      gfx->drawRect(mark[i] - 2, locHum[i] - 2, markSize, markSize, pointLineColor);
    }
  }
  //draw some time stamps
  for (int i = 0; i < NumPoints; i = i + NumPoints / 4) {
    gfx->drawLine(mark[i], originY, mark[i], minorSizeY, graphColor2);
    PrintCharTFT(DataTimeStr[i][1], (mark[i] - 20), (originY + 16), graphColor2, BackgroundColor, 1);
    PrintCharTFT(DataTimeStr[i][0], (mark[i] - 20), (originY + 26), graphColor, BackgroundColor, 1);
  }
  // draw all the lines
  if (pointLines == true) {
    for (int i = 0; i < (NumPoints - 1); i++)
    {
      gfx->drawLine(mark[i], locTemp[i], mark[i + 1], locTemp[i + 1], lineColor);
      gfx->drawLine(mark[i], locHum[i], mark[i + 1], locHum[i + 1], lineColor2);
    }
  }
}

int LoadFileMatrixDHT() {
  fs::File fileTest = LITTLEFS.open("/dataDHT.csv");
  if (!fileTest) {
    Serial.println("Failed to open dataDHT for reading");
    return 0;
  }
  //read file and make it a vector
  vector<String> v;
  while (fileTest.available()) {
    v.push_back(fileTest.readStringUntil('\n'));
  }
  fileTest.close();

  int NumRowsFile = v.size();
  Serial.print("Total number lines in dataDHT file: "); Serial.println(NumRowsFile);
  Serial.println("----Printing vector v dataDHT----");

  //check if file is large or small compared to numXticks

  int Interval; int LastLinetoLoad;
  if ((NumRowsFile / numXticks) > 3) {
    Serial.println("Large DHT file!");
    Interval = NumRowsFile / (numXticks-1);
    Serial.print("Interval: ");Serial.println(Interval);
    LastLinetoLoad = max(0,NumRowsFile - Interval*(numXticks-1) -2);
  }
  else {
    Serial.println("Small DHT file!");
    Interval = 1;
    LastLinetoLoad = max(0, (NumRowsFile - numXticks - 1)); //first line should be headers
  }
  int k = 0;
  
  Serial.print("LastLinetoLoad :"); Serial.println(LastLinetoLoad);
  for (int i = NumRowsFile - 1 ; i > LastLinetoLoad; i = i - Interval) {
    Serial.print("Line "); Serial.print(i); Serial.print(": "); Serial.println(v[i]);
    char Buff[100];
    v[i].toCharArray(Buff, 100);  //change from string to char
    char* token;
    char* rest = Buff;
    int j = 0;
    while ((token = strtok_r(rest, ",", &rest))) {
      if (j < 2) { //first two columns are date and time
        DataTimeStr[k][j] = token;
      }
      else if (j == 2) { //Temp
        Data[k][j - 2] = strtod(token, NULL); //convert string to double
        Tmin = min(Tmin, Data[k][j - 2]);
        Tmax = max(Tmax, Data[k][j - 2]);
      }
      else if (j == 3) { //Hum
        Data[k][j - 2] = strtod(token, NULL); //convert string to double
        Hmin = min(Hmin, Data[k][j - 2]);
        Hmax = max(Hmax, Data[k][j - 2]);
      }
      j++;
    }
    k++;
  }
  Serial.print("Tmin/Tmax: "); Serial.print(Tmin); Serial.print(' '); Serial.println(Tmax);
  Serial.print("Hmin/Hmax: "); Serial.print(Hmin); Serial.print(' '); Serial.println(Hmax);
  //    Serial.println();
  //      for (int i = NumRowsFile -1; i > LastLinetoLoad; i--) {
  //    Serial.print("i: "); Serial.print(i); Serial.print("; ");
  //    for (int j = 0; j < 2; j++) {
  //      if (j) {
  //        Serial.print(' ');
  //      }
  //      Serial.print(Data[i][j]);
  //    }
  //    Serial.println();
  //  }
  Serial.println("Done importing DHT file");
  //  Serial.println("Done loading Data file");
  return min(NumRowsFile - 1, numXticks);
}
