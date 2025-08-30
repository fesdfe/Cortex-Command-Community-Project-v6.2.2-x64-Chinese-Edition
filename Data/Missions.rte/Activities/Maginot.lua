package.loaded.Constants = nil;
require("Constants");

function MaginotMission:StartActivity(isNewGame)
	self.defenderLZ = SceneMan.Scene:GetArea("LZ Team 1");
	self.attackLZ1 = SceneMan.Scene:GetArea("LZ Team 2");
	self.attackLZ2 = SceneMan.Scene:GetArea("Enemy Sneak Spawn 2");
	self.maginotBunkerArea = SceneMan.Scene:GetArea("Maginot Bunker");
	self.evacuateTrigger = SceneMan.Scene:GetArea("Evacuate Trigger");
	self.rescueTrigger = SceneMan.Scene:GetArea("Rescue Trigger");
	self.rescueLZ = SceneMan.Scene:GetArea("Rescue Area");

	self.fightStage = { beginFight = 0, defendLeft = 1, defendRight = 2, evacuateBrain = 3, enterEvacuationRocket = 4 };

	self.attackerTeam = Activity.TEAM_1;
	self.defenderTeam = Activity.TEAM_2;
	self.attackerTech = self:GetTeamTech(self.attackerTeam);
	self.defenderTech = self:GetTeamTech(self.defenderTeam);
	
	self:SetLZArea(self.defenderTeam, self.defenderLZ);

	self.screenTextTimer = Timer();
	self.screenTextTimeLimit = 7500;
	self.roundTimer = Timer();
	self.spawnTimer = Timer();
	self.evacuateCheckTimer = Timer(2500);

	self.numberOfAttackersPerCraft = math.ceil(2 * (self.Difficulty / Activity.MEDIUMDIFFICULTY));

	self.brainDead = {};

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function MaginotMission:OnSave()
	self:SaveNumber("screenTextTimer.ElapsedSimTimeMS", self.screenTextTimer.ElapsedSimTimeMS);
	self:SaveNumber("roundTimer.ElapsedSimTimeMS", self.roundTimer.ElapsedSimTimeMS);
	self:SaveNumber("spawnTimer.ElapsedSimTimeMS", self.spawnTimer.ElapsedSimTimeMS);

	self:SaveNumber("currentFightStage", self.currentFightStage);
	self:SaveNumber("evacuationRocketSpawned", self.evacuationRocketSpawned and 1 or 0);

	self:SaveNumber("spawnTime", self.spawnTime);
end

function MaginotMission:StartNewGame()
	self:SetTeamFunds(self:GetStartingGold(), self.defenderTeam);

	self.currentFightStage = self.fightStage.beginFight;
	self.evacuationRocketSpawned = false;

	self.spawnTime = 30000 * math.exp(self.Difficulty * -0.014) * rte.SpawnIntervalScale; -- Scale spawn time from 20s to 5s. Normal = 10s

	if self:GetFogOfWarEnabled() then
		SceneMan:MakeAllUnseen(Vector(20, 20), self.defenderTeam);
		for maginotBunkerAreaBox in self.maginotBunkerArea.Boxes do
			SceneMan:RevealUnseenBox(maginotBunkerAreaBox.Corner.X, maginotBunkerAreaBox.Corner.Y, maginotBunkerAreaBox.Width, maginotBunkerAreaBox.Height, self.defenderTeam);
		end
	end

	for actor in MovableMan.AddedActors do
		actor.AIMode = Actor.AIMODE_SENTRY;
	end

	self:SetupHumanPlayerBrains();

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and not self:GetPlayerBrain(player) then
			self:ResetMessageTimer(player);
			FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
		end
	end
end

function MaginotMission:SetupHumanPlayerBrains()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and not self:GetPlayerBrain(player) then
			self.brainDead[player] = false;
			local brain = CreateActor("Brain Case", "Base.rte");
			brain.Team = self.defenderTeam;
			brain.Pos = Vector(2328 + (player * 24), 1170);
			self:SetPlayerBrain(brain, player);
			self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
			MovableMan:AddActor(brain);
			-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
			self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);

			local emptyBrainBody = CreateAHuman("Brain Robot", "Base.rte");
			emptyBrainBody:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.defenderTech));
			emptyBrainBody:SetAimAngle(-0.78)
			emptyBrainBody.HUDVisible = false;
			emptyBrainBody.PlayerControllable = false;
			emptyBrainBody.Pos = brain.Pos + Vector(0, 65);
			emptyBrainBody.Head.Scale = 0;
			emptyBrainBody.Team = brain.Team;
			emptyBrainBody.PinStrength = 100;
			MovableMan:AddActor(emptyBrainBody);
			emptyBrainBody:GetController().InputMode = Controller.CIM_DISABLED;
		end
	end
end

function MaginotMission:ResumeLoadedGame()
	self.screenTextTimer.ElapsedSimTimeMS = self:LoadNumber("screenTextTimer.ElapsedSimTimeMS");
	self.roundTimer.ElapsedSimTimeMS = self:LoadNumber("roundTimer.ElapsedSimTimeMS");
	self.spawnTimer.ElapsedSimTimeMS = self:LoadNumber("spawnTimer.ElapsedSimTimeMS");

	self.currentFightStage = self:LoadNumber("currentFightStage");
	self.evacuationRocketSpawned = self:LoadNumber("evacuationRocketSpawned") ~= 0;

	self.spawnTime = self:LoadNumber("spawnTime");

	if self.evacuationRocketSpawned then
		for actor in MovableMan.AddedActors do
			if actor.Team == self.defenderTeam and actor.PresetName == "Rocket MK2" and self.rescueLZ:IsInside(Vector(actor.Pos.X, self.rescueLZ:GetCenterPoint().Y)) then
				self.evacuationRocket = ToACRocket(actor);
				break;
			end
		end
	end
end

function MaginotMission:EndActivity()
	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the ingame settings.
	if not self:IsPaused() then
		-- Play sad music if no humans are left
		if self:HumanBrainCount() == 0 then
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		else
			-- But if humans are left, then play happy music!
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		end
	end
end

function MaginotMission:DoGameOverCheck()
	if self.WinnerTeam ~= self.defenderTeam then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				local team = self:GetTeamOfPlayer(player);

				if self.ActivityState ~= Activity.OVER then
					local brain = self:GetPlayerBrain(player);
					if not brain or not MovableMan:IsActor(brain) or not brain:HasObjectInGroup("Brains") then
						self:SetPlayerBrain(nil, player);
						local newBrain = MovableMan:GetUnassignedBrain(team);
						if newBrain and not self.brainDead[player] then
							self:SetPlayerBrain(newBrain, player);
							if MovableMan:IsActor(newBrain) then
								self:SwitchToActor(newBrain, player, team);
							end
							self:GetBanner(GUIBanner.RED, player):ClearText();
						else
							FrameMan:SetScreenText("你失去主脑了!", self:ScreenOfPlayer(player), 333, -1, false);
							self.brainDead[player] = true;

							local gameOver = true;
							for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
								if self:PlayerActive(player) and self:PlayerHuman(player) and not self.brainDead[player] then
									gameOver = false;
									break;
								end
							end

							if gameOver then
								self.WinnerTeam = self:OtherTeam(team);
								ActivityMan:EndActivity();
							end
							self:ResetMessageTimer(player);
						end
					else
						if self.evacuationRocket and brain.UniqueID ~= self.evacuationRocket.UniqueID then
							self:AddObjectivePoint("保护!", brain.AboveHUDPos, self.defenderTeam, GameActivity.ARROWDOWN);
						end

						self:SetObservationTarget(brain.Pos, player);
					end
				end
			end
		end
	elseif self.WinnerTeam == self.defenderTeam then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				self:GetBanner(GUIBanner.RED, player):ClearText();

				if not self.GameOverTimer:IsPastSimMS(self.GameOverPeriod) then
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					if self.brainDead[player] then
						FrameMan:SetScreenText("你或许已逝,但你的同类们将继续生存,为未来而战.放心,你一定会得到复仇!", self:ScreenOfPlayer(player), 0, 1, false);
					else
						FrameMan:SetScreenText("干得好,你又活过了一天!我们已经找到了敌人的基地,正在计划对其进行突袭!", self:ScreenOfPlayer(player), 0, 1, false);
					end
				else
					ActivityMan:EndActivity();
				end
			end
		end
	end
end

function MaginotMission:SwapBrainsToRobots()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and self:GetPlayerBrain(player) and not self.brainDead[player] then
			local brainCase = self:GetPlayerBrain(player);
			local nearestBrainRobot = MovableMan:GetClosestTeamActor(brainCase.Team, player, brainCase.Pos, 100, Vector(), brainCase);

			if nearestBrainRobot then
				nearestBrainRobot = ToAHuman(nearestBrainRobot);
				nearestBrainRobot.Head.Scale = 1;
				nearestBrainRobot.HUDVisible = true;
				nearestBrainRobot.PlayerControllable = true;
				nearestBrainRobot:GetController().InputMode = Controller.CIM_AI;
				nearestBrainRobot.PinStrength = 0;
				self:SetPlayerBrain(nearestBrainRobot, player);

				brainCase.ToDelete = true;
			end
		end
	end
end

function MaginotMission:SpawnEvacuationRocket()
	self.evacuationRocket = CreateACRocket("Rocket MK2", "Base.rte");
	self.evacuationRocket.Pos = Vector(self.rescueLZ:GetCenterPoint().X, -100);
	self.evacuationRocket.Team = self.defenderTeam;
	self.evacuationRocket:SetControllerMode(Controller.CIM_AI, -1);
	self.evacuationRocket.HUDVisible = false;
	self.evacuationRocket.PlayerControllable = false;
	self.evacuationRocket.AIMode = Actor.AIMODE_STAY;
	self.evacuationRocket:SetGoldValue(0);
	MovableMan:AddActor(self.evacuationRocket);
	self.evacuationRocketSpawned = true;
end

function MaginotMission:UpdateScreenText()
	if not self.screenTextTimer:IsPastSimMS(self.screenTextTimeLimit) then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
				if self.currentFightStage == self.fightStage.beginFight then
					if self:GetTeamFunds(self.defenderTeam) == 0 then
						FrameMan:SetScreenText("传感器显示,敌方空投船正在前往左门的途中.\n你只能利用现场现有的兵力了.祝你好运!", self:ScreenOfPlayer(player), 0, 1, false);
					else
						FrameMan:SetScreenText("传感器显示,敌方空投船正在前往左门的途中.\n尽快做好准备,他们很快就会到!", self:ScreenOfPlayer(player), 0, 1, false);
					end
				elseif self.currentFightStage == self.fightStage.defendLeft then
					FrameMan:SetScreenText("攻势已经开始,坚守阵地!", self:ScreenOfPlayer(player), 1500, 1, true);
				elseif self.currentFightStage == self.fightStage.defendRight then
					FrameMan:SetScreenText("警报:一支地面攻击部队正往右门入口移动!", self:ScreenOfPlayer(player), 1500, 1, true);
				elseif self.currentFightStage == self.fightStage.evacuateBrain then
					if self.PlayerCount == 1 then
						FrameMan:SetScreenText("敌军势力过于强大,立即撤离掩体!\n你的主脑已被上传至机器人,前往着陆区撤离.", self:ScreenOfPlayer(player), 0, 1, true);
					else
						FrameMan:SetScreenText("敌军势力过于强大,立即撤离掩体!\n你的主脑已加载到机器人上,尽可能地将它运至着陆区并撤离.", self:ScreenOfPlayer(player), 0, 1, true);
					end
				elseif self.currentFightStage == self.fightStage.enterEvacuationRocket then
					FrameMan:SetScreenText("撤离火箭即将到达,快到着陆区!", self:ScreenOfPlayer(player), 1500, 1, true);
				end
			end
		end
	end
end

function MaginotMission:UpdateAttackerSpawns()
	if self.spawnTimer:IsPastSimMS(self.spawnTime) then
		self.spawnTimer:Reset();

		local createAttackerAHuman = function(self, attackerIsHeavyActor)
			local attackerAHuman = RandomAHuman(attackerIsHeavyActor and "Actors - Heavy" or "Actors - Light", self.attackerTech);
			if attackerAHuman then
				local mainWeaponIsHeavy = math.random() < (attackerIsHeavyActor and 0.65 or 0.35);
				attackerAHuman:AddInventoryItem(RandomHDFirearm(mainWeaponIsHeavy and "Weapons - Heavy" or "Weapons - Primary", self.attackerTech));

				local secondaryEquipmentIsGrenade = math.random() < 0.33;
				if secondaryEquipmentIsGrenade then
					attackerAHuman:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.attackerTech));
				else
					local secondaryEquipmentIsLightWeapon = mainWeaponIsHeavy;
					if math.random() < 0.2 then
						secondaryEquipmentIsLightWeapon = not secondaryEquipmentIsLightWeapon;
					end
					attackerAHuman:AddInventoryItem(RandomHDFirearm(secondaryEquipmentIsLightWeapon and "Weapons - Light" or "Weapons - Secondary", self.attackerTech));
					if not secondaryEquipmentIsLightWeapon and math.random() < 0.33 then
						attackerAHuman:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.attackerTech));
					end
				end
				if math.random() < 0.33 then
					if math.random() < 0.75 then
						attackerAHuman:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.attackerTech));
					else
						attackerAHuman:AddInventoryItem(RandomTDExplosive("Tools - Breaching", self.attackerTech));
					end
				end
				attackerAHuman.Team = self.attackerTeam;
				attackerAHuman.AIMode = Actor.AIMODE_BRAINHUNT;
			end
			return attackerAHuman;
		end

		if self.currentFightStage >= self.fightStage.defendLeft then
			local attackerCraft = RandomACDropShip("Craft", self.attackerTech);
			if not attackerCraft then
				attackerCraft = CreateACDropship("Dropship MK1", "Base.rte");
			end

			if attackerCraft then
				for i = 1, math.min(self.numberOfAttackersPerCraft, attackerCraft.MaxPassengers) do
					local attackerIsHeavyActor = self.currentFightStage >= self.fightStage.defendRight and math.random() < 0.5;
					attackerCraft:AddInventoryItem(createAttackerAHuman(self, attackerIsHeavyActor));
				end

				if self.attackLZ1 then
					attackerCraft.Pos = Vector(self.attackLZ1:GetRandomPoint().X, -50);
				else
					attackerCraft.Pos = Vector(SceneMan.Scene.Width * PosRand(), -50);
				end

				attackerCraft.Team = self.attackerTeam;
				attackerCraft:SetControllerMode(Controller.CIM_AI, -1);
				MovableMan:AddActor(attackerCraft);
			end
		end

		if self.currentFightStage >= self.fightStage.defendRight then
			local attackerActor;
			if math.random() < self:GetCrabToHumanSpawnRatio(PresetMan:GetModuleID(self.attackerTech)) then
				attackerActor = RandomACrab("Actors - Mecha", self.attackerTech);
				if attackerActor then
					attackerActor.Team = self.attackerTeam;
					attackerActor.AIMode = Actor.AIMODE_BRAINHUNT;
				end
			else
				attackerActor = createAttackerAHuman(self, math.random() < 0.5);
			end
			if attackerActor then
				for attackerLZ2Box in self.attackLZ2.Boxes do
					attackerActor.Pos = Vector(attackerLZ2Box.Corner.X + attackerLZ2Box.Width, attackerLZ2Box.Center.Y); -- Assumes only one box in this Area. If there are more boxes, these spawns may be a little odd.
					break;
				end
				MovableMan:AddActor(attackerActor);
			end
		end
	end
end

function MaginotMission:UpdateActivity()
	self:ClearObjectivePoints();

	if (self.ActivityState == Activity.OVER) then
		return;
	end

	self:DoGameOverCheck();
	
	local enemyInsideBrainEvacuateArea = false;
	if self.currentFightStage < self.fightStage.evacuateBrain and self.evacuateCheckTimer:IsPastSimTimeLimit() then
		for movableObject in MovableMan:GetMOsInBox(self.evacuateTrigger.FirstBox, self.defenderTeam, true) do
			if IsAHuman(movableObject) or IsACrab(movableObject) then
				enemyInsideBrainEvacuateArea = true;
				break;
			end
		end
		self.evacuateCheckTimer:Reset();
	end

	local difficultyTimeMultiplier = math.max(0.5, self.Difficulty / Activity.MEDIUMDIFFICULTY);
	if self.roundTimer:IsPastSimMS(math.min(15000, 20000 / difficultyTimeMultiplier)) and self.currentFightStage == self.fightStage.beginFight then
		self.currentFightStage = self.fightStage.defendLeft;
		self.screenTextTimer:Reset();
		self.roundTimer:Reset();
	elseif self.roundTimer:IsPastSimMS(240000 * difficultyTimeMultiplier) and self.currentFightStage == self.fightStage.defendLeft then
		self.currentFightStage = self.fightStage.defendRight;
		self.screenTextTimer:Reset();
		self.roundTimer:Reset();
	elseif enemyInsideBrainEvacuateArea or (self.roundTimer:IsPastSimMS(180000 * difficultyTimeMultiplier) and self.currentFightStage == self.fightStage.defendRight) then
		self:SwapBrainsToRobots();
		self.currentFightStage = self.fightStage.evacuateBrain;
		self.screenTextTimer:Reset();
		self.roundTimer:Reset();
	elseif self.currentFightStage < self.fightStage.enterEvacuationRocket then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) and not self.brainDead[player] and self.rescueTrigger:IsInside(self:GetPlayerBrain(player).Pos) then
				self:SpawnEvacuationRocket();
				self.currentFightStage = self.fightStage.enterEvacuationRocket;
				self.screenTextTimer:Reset();
				self.roundTimer:Reset();
				break;
			end
		end
	end

	self:UpdateScreenText();

	self:UpdateAttackerSpawns();

	if self.currentFightStage == self.fightStage.evacuateBrain then
		self:AddObjectivePoint("到达着陆区!", self.rescueLZ:GetCenterPoint() + Vector(0, 20), self.defenderTeam, GameActivity.ARROWDOWN);
	elseif self.currentFightStage == self.fightStage.enterEvacuationRocket then
		if not self.evacuationRocket or not MovableMan:IsActor(self.evacuationRocket) then
			self.evacuationRocket = nil;
		elseif self.evacuationRocket then
			local evacuationRocketHasAllBrains = true;
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) and not self.brainDead[player] then
					local brain = self:GetPlayerBrain(player);
					if brain and brain.UniqueID ~= self.evacuationRocket.UniqueID then
						evacuationRocketHasAllBrains = false;
						break;
					end
				end
			end
			if not evacuationRocketHasAllBrains then
				self:AddObjectivePoint("进入火箭!", self.evacuationRocket.AboveHUDPos, self.defenderTeam, GameActivity.ARROWDOWN);
				self.evacuationRocket:OpenHatch();
			else
				self.evacuationRocket.AIMode = Actor.AIMODE_RETURN;
			end
		end
	end

	self:YSortObjectivePoints();
end


function MaginotMission:CraftEnteredOrbit(orbitedCraft)
	if orbitedCraft:HasObjectInGroup("Brains") then
		self.WinnerTeam = self.defenderTeam;
		self.GameOverTimer:Reset();

		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			self:SetPlayerBrain(nil, player);
		end
	end
end