var hcal = require('./build/Debug/hcal');

console.log('\n\n!#----------Test results----------#!\n');
var d1 = new Date(2013, 0, 01, 08),
    d2 = new Date(2013, 1, 28, 23),
    d3 = new Date(2013, 1, 22, 23, 30),
    d4 = new Date(2013, 1, 23, 0, 30),
    d5 = new Date(2013, 0, 13, 16),
    d6 = new Date(2013, 0, 13, 18),
    d7 = new Date(2012, 11, 30, 10),
    d8 = new Date(2012, 11, 30, 11),
    d9 = new Date(2012, 11, 30),
    d10 = new Date(2013, 0, 3, 10),
    d11 = new Date(2013, 0, 3, 11);


var a = hcal.createEvent(1, d1, d2, 96, 1, "Computers");
// var b = hcal.createEvent(2, d3, d4, 13, "Betsy", "Basketball");
// var c = hcal.createEvent(2, d5, d6, 13, "Reginald", "Cartography");
// var events = [a, b, c];
// var ret = hcal.testEventArray(events);
// console.log(ret[3].description());
// console.log("second event duration: " + b.duration() + " minutes");

console.log(d3 + '\n');
console.log(d4);
hcal.insertEvent(d10, d11, 4, 2, "Reptile Crochet", true, function(err, evt){
    if (!err){
        console.log(evt.description() + " - " + evt.end());
    }
    else{
        console.log("insert event error: " + err);
    }
});
// hcal.updateEvent(35, d5, d6, 5, 1, "New title", false, function(err, evt){
//     console.log("updated event description: " + evt.description());
// });
hcal.updateEvent(83, d5, d6, 5, 1, "New title and I love being here...", false, function(err){
    if (!err){
        console.log("event updated successfully.");
    }
    else{
        console.log(err);
    }   
});
hcal.printCalendar(d1, d2, "month", function(fileName){
    console.log("Monthly calendar file: " + fileName);
});
hcal.printCalendar(d9, d2, "week", function(fileName){
    console.log("Weekly calendar file: " + fileName);
});
hcal.getEvents(d1, d3, function(err, evts){
    if (err){
        console.log(err);
    }
    else{
        var len = evts.length, i = 0;
        for (; i < len; ++i){
            console.log(evts[i].description() + " - " + evts[i].start());
        }
    }
});
//var cfg = hcal.createConfig(1, 5, 8, 20);
//console.log(notRet);