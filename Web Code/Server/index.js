// set up express
const express = require("express");
const path = require("path");
const app = express();

//setup bootstrap 
app.use('/css', express.static(path.join(__dirname, 'node_modules/bootstrap/dist/css')));
app.use('/js', express.static(path.join(__dirname, 'node_modules/bootstrap/dist/js')));

// connect to mongo
const dotenv = require('dotenv').config()
const mongoUsername = process.env.mongoUsername
const mongoPassword = process.env.mongoPassword
const mongoAppName = process.env.mongoAppName

const DBConnection = `mongodb+srv://${mongoUsername}:${mongoPassword}@mysticreader.u8qzncq.mongodb.net/${mongoAppName}?retryWrites=true&w=majority`
const mongoose = require("mongoose");
const session = require("express-session");
mongoose.connect(DBConnection)

// DB models
const Users = require("./models/users.js");
const Data = require("./models/boxData.js");
const { request } = require("http");


// Set up a express session
app.use(session({
    secret: "KEY",
    resave: false,
    saveUninitialized: false,
    cookie: { maxAge: 5 * 60 * 1000 }
}))

//serve files from /public 
app.use(express.static('public'))

// allow form data to be used
app.use(express.urlencoded({ extended: false }))

// allow json data to be recieved
app.use(express.json());

const port = process.env.PORT || 3000; // set like this to work with Render
app.listen(port, () => console.log(`Server running on ${port}`));

// function to check if the user is logged in before letting them access  
function checkIfLoggedIn(request, response, nextAction) {
    if (request.session) {
        if (request.session.username) {
            nextAction()
        } else {
            request.session.destroy()
            response.sendFile(path.join(__dirname, 'views', 'welcome.html'))
        }
    }
    // if not logged in, send to notLogged
    else {
        request.session.destroy()
        response.sendFile(path.join(__dirname, 'views', 'welcome.html'))
    }
}

// post request when user submits login form
app.post(`/login`, async (request, response) => {
    const userData = await Users.checkUser(request.body.username, request.body.password)
    if (userData) {
        console.log("Login successful")
        request.session.username = request.body.username
        request.session.boxID = userData.boxID
        //console.log(req.session.boxID)
        response.redirect("/app") // use redirect to go from modal to app page
    } else {
        console.log("Login failed")
        response.sendFile(path.join(__dirname, '/views', 'welcome.html'))
    }
})


app.post('/register', async (request, response) => {
    const success = await Users.addNewUser(request.body.username, request.body.password, request.body.boxID);
    if (success) {
        request.session.username = request.body.username
        request.session.boxID = request.body.boxID
        response.redirect("/app") // use redirect to go from modal to app page
    } else {
        response.sendFile(path.join(__dirname, '/views', 'welcome.html'))
    }
})



// user requests /login end point
app.get('/welcome', (request, response) => {
    response.sendFile(path.join(__dirname, '/views', 'welcome.html'))
})

// user requests /app end point
app.get('/app', checkIfLoggedIn, (req, res) => {
    res.sendFile(path.join(__dirname, '/views', 'app.html'))
});

//user is in /app and wants data
app.get ('/priorReadings', async (request, response)=>{
    response.json({dataEntries: await Data.getDataFromDB(request.session.boxID)})
})

//user is in /app and wants FIRST data point
app.get ('/mostRecentReading', async (request, response)=>{
    response.json({dataEntriesOne: await Data.getOneDataFromDB(request.session.boxID)})
})

//user wants card images
app.get('/image/:id', (request, response)=>{
    const id = request.params.id
    const filePath = path.join(__dirname, 'Assets', 'Cards-png', `${id}.png`)
    console.log(__dirname)
    response.sendFile(filePath)
})


// Data is sent from ESP32
app.post("/data", async (req, res) => {
    console.log(req.body);

    //Send data to mongo, store in db 
    //it comes in as 
    //req.body = { box_id: 'TEST123', card_id: [ 12, 0, 0, 0, 0 ] }

    let sendData = await Data.addNewBoxData(req.body.box_id, req.body.card_id );
    if(sendData){
        console.log("Saved data")
        res.status(200).send("Recieved Data!")
    }
    else{
        console.log("Couldnt Save Data!")
        res.status(500).send("Recieved Data!")
    }
});
