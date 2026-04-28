const mongoose=require("mongoose")
const {Schema, model} = mongoose

const dataFromDBSchema = new Schema({
    boxID: String,
    data : [Number],
})

const dataFromDB = model("data", dataFromDBSchema)

//add box data to database 
async function getDataFromDB(){
    let recievedData=[]
    recievedData = await dataFromDB.find({})
    return recievedData
}


module.exports={
    getDataFromDB
}