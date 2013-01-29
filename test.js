var hcal = require('./build/Release/hcal');

console.log('\n\n!#----------Test results----------#!\n');
var d1 = new Date(2013, 0, 13, 13);
var d2 = new Date(2013, 0, 13, 14);
var d3 = new Date(2013, 1, 22, 23, 30);
var d4 = new Date(2013, 1, 23, 0, 30);
var d5 = new Date(2013, 0, 13, 16);
var d6 = new Date(2013, 0, 13, 18);

var a = hcal.createEvent(1, d1, d2, 96, "Bob", "Computers");
var b = hcal.createEvent(2, d3, d4, 13, "Betsy", "Basketball");
var c = hcal.createEvent(2, d5, d6, 13, "Reginald", "Cartography");
var events = [a, b, c];
var ret = hcal.testEventArray(events);
//var notRet = hcal.isJsArray();
console.log(ret);
console.log("First event duration: " + a.duration() + " minutes");

hcal.buildCalendar(events, "month", function(fileName){
    console.log("Calendar file: " + fileName);    
});
//console.log(notRet);