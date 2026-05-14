const mongoose=require("mongoose")
const {Schema, model} = mongoose


const userSchema = new Schema({
    username: String,
    password: String,
    boxID: String,
})
const newBoxSchema = new Schema({
    username: String,
    boxID: String,
})

const userData = model("users", userSchema)
const newBox = model("currentboxes", newBoxSchema)

//return bool if user is there
async function checkUser(username, password){
 
    let found = null
    found = await userData.findOne({username:username}).exec()

    //checks password - if it is same then it will return true
    if(found && found.password==password){
        return found
    } else{
        return false
    }
}

//add user to database
async function addNewUser(username, password, boxID){
 
    
    //check database to see if user already exists
    let findUser = await userData.findOne({username:username}).exec()

    if(findUser){
        return false
    }
    else{
        let newUserData = {
            username: username,
            password: password, 
            boxID: boxID
        }
        await userData.create(newUserData)

        // add box to currentBoxes
        let newBoxData = {
            username: username,
            boxID: boxID
        }
        await newBox.create(newBoxData)
        return true
    }
}

module.exports={
    checkUser,
    addNewUser
}